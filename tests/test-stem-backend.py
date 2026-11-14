"""Backend failure tests, independent of foobar and of model downloads."""
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch
import torch
import demucs.apply

spec = importlib.util.spec_from_file_location('worker', Path(__file__).parents[1]/'stems/stems_worker.py')
worker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(worker)

class Model:
    sources = list(worker.NAMES)

class Backends(unittest.TestCase):
    def check(self, available, fail=False, cpu_fail=False):
        calls = []
        def apply(model, audio, **kwargs):
            calls.append(kwargs['device'])
            if kwargs['device'] == 'cuda' and fail:
                raise RuntimeError('simulated CUDA out of memory')
            if kwargs['device'] == 'cpu' and cpu_fail:
                raise RuntimeError('simulated CPU failure')
            return torch.zeros(1, 4, 2, 32)
        with patch.object(torch.cuda, 'is_available', return_value=available), \
             patch.object(torch.cuda, 'synchronize'), patch.object(torch.cuda, 'empty_cache'), \
             patch.object(worker, 'load_model', side_effect=Model) as load, \
             patch.object(demucs.apply, 'apply_model', side_effect=apply):
            if cpu_fail:
                with self.assertRaisesRegex(RuntimeError, 'CPU failure'):
                    worker.run_inference(torch.zeros(2, 32), lambda *args: None)
            else:
                result, backend, sources = worker.run_inference(torch.zeros(2, 32), lambda *args: None)
                self.assertEqual(result.shape, (4, 2, 32))
                self.assertEqual(backend, 'cuda' if available and not fail else 'cpu')
                self.assertEqual(sources, list(worker.NAMES))
            self.assertEqual(calls, ['cuda','cpu'] if available and fail else ['cuda'] if available else ['cpu'])
            self.assertEqual(load.call_count, 2 if available and fail else 1)

    def test_no_gpu(self): self.check(False)
    def test_gpu(self): self.check(True)
    def test_gpu_failure(self): self.check(True, fail=True)
    def test_cpu_error_propagates(self): self.check(False, cpu_fail=True)
    def test_both_fail(self): self.check(True, fail=True, cpu_fail=True)

if __name__ == '__main__': unittest.main()
