#include "stdafx.h"

#include <ole2.h>
#include <atlimage.h>
#include <SDK/album_art.h>
#include <shellapi.h>
#include <string>
#include <exception>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <locale>

#include <libPPUI/win32_op.h>
#include <helpers/BumpableElem.h>
#include <helpers/input_helpers.h>

#include "foo_dj_version.h"
#include "webview_drop.h"
#include "tempo_match.h"
#include "musical_key.h"
#include "beat_sync.h"
#include "dj_language.h"
#include "dj_skin.h"
#include "deck_queue.h"
#include "queue_format.h"
#include "queue_duration_hook.h"
#include "auto_mix.h"
#include "master_limiter.h"
#include "bpm_provider.h"
#include "output_rate.h"
#include "midi_control.h"
#include "midi_input.h"
#include "queue_window.h"
#include "settings_window.h"
#include "tooltip_policy.h"
#include "session_bundle_io.h"
#include <commdlg.h>
#pragma comment(lib,"comdlg32.lib")
#include "stem_cache.h"
#include "stem_job.h"
#include "stem_cache_manager.h"
#include "listening_policy.h"
#include "cue_output.h"
#include "preview_plan.h"
#include "preview_window.h"
#include "preview_overview.h"
#include "silence_trim.h"
#include "ab_loop.h"
#include "loop_storage.h"
#include "scrobble_bridge.h"
#include <ctime>
#include <commctrl.h>
#include "third_party/soundtouch/include/SoundTouch.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

// foo_dj - etape 4 : volume par deck, forme d'onde avec zoom, positionnement dans le morceau.
// (par-dessus l'etape 3 : crossfader, et l'etape 2 : lecture avec format de sortie fixe).
//
// Chaque deck a : un titre, le nom du fichier, le temps, une forme d'onde (cliquable pour
// se deplacer dans le morceau, molette ou boutons - / + pour zoomer), un curseur de volume,
// et deux boutons : Lecture/Pause, Debut. Les morceaux se chargent par glisser-deposer.
// Un petit "moteur" tourne dans un thread a part : il decode les fichiers des decks qui jouent,
// les convertit en stereo a la frequence de la sortie audio, les melange avec le crossfader
// et les volumes, puis envoie le tout a la sortie audio configuree dans foobar2000.
// La forme d'onde de chaque fichier est calculee en arriere-plan juste apres son chargement.

namespace {
	// Notre identifiant unique (GUID) pour le panneau. Ne pas le reutiliser ailleurs.
	static const GUID guid_dj_panel = { 0x7a1f3c52, 0x9d4e, 0x4b6a, { 0x8e, 0x21, 0x5c, 0x0d, 0x93, 0xb7, 0xf4, 0xa8 } };

	// One persisted session for the two platines; foobar owns storage and flushing.
	static cfg_string cfg_session({0x51e84a78,0x0b91,0x48a7,{0x88,0x44,0xe1,0xb6,0x22,0x18,0xd9,0x02}}, "");
	static cfg_int cfg_language({0x33565be2,0x5eda,0x405f,{0xa8,0x54,0x72,0x6a,0xe7,0x81,0x18,0x9d}},0);
	static cfg_int cfg_flat({0x49961560,0xb69e,0x4070,{0xa7,0x31,0x15,0x7c,0xc8,0xa2,0x43,0x21}},0);
    static cfg_int cfg_colorA({0x49961561,0xb69e,0x4070,{0xa7,0x31,0x15,0x7c,0xc8,0xa2,0x43,0x21}},0);
    static cfg_int cfg_colorB({0x49961562,0xb69e,0x4070,{0xa7,0x31,0x15,0x7c,0xc8,0xa2,0x43,0x21}},1);
    static cfg_int cfg_replaygain({0xa660da20,0x74ab,0x44f7,{0x95,0x4a,0x62,0xc3,0x92,0x13,0x51,0x03}},0);
    static cfg_int cfg_midiEnabled({0x6139a101,0x71be,0x4ca3,{0x91,0x39,0,0,0,0,1,1}},0);
    static cfg_int cfg_midiPickup({0x6139a102,0x71be,0x4ca3,{0x91,0x39,0,0,0,0,1,2}},1);
    static cfg_string cfg_midiDevice({0x6139a103,0x71be,0x4ca3,{0x91,0x39,0,0,0,0,1,3}},"");
    static cfg_string cfg_midiMappings({0x6139a104,0x71be,0x4ca3,{0x91,0x39,0,0,0,0,1,4}},"");
    static cfg_int cfg_outputRate({0x6139a004,0x71be,0x4ca3,{0x91,0x39,0x00,0x00,0x00,0x00,0x00,0x04}},0);
    static cfg_int cfg_bpmProvider({0x6139a003,0x71be,0x4ca3,{0x91,0x39,0x00,0x00,0x00,0x00,0x00,0x03}},0);
    static cfg_int cfg_masterLimiter({0x6139a001,0x71be,0x4ca3,{0x91,0x39,0x00,0x00,0x00,0x00,0x00,0x01}},1);
    static cfg_int cfg_masterCeiling({0x6139a002,0x71be,0x4ca3,{0x91,0x39,0x00,0x00,0x00,0x00,0x00,0x02}},-10);
    static cfg_int cfg_scrobble({0xa660da21,0x74ab,0x44f7,{0x95,0x4a,0x62,0xc3,0x92,0x13,0x51,0x03}},0);
    static std::atomic<int> g_replaygain{0};
    static std::atomic<bool> g_scrobble{false},g_scrobbleReady{false};
    static cfg_string cfg_queueFormat({0xbe993a10,0x366b,0x45b7,{0x9a,0x14,0x3a,0x6d,0x81,0xb2,0x11,0x5e}},"%artist% - %title% '('%length%')'");
    static cfg_int cfg_queueNumbers({0xbe993a11,0x366b,0x45b7,{0x9a,0x14,0x3a,0x6d,0x81,0xb2,0x11,0x5e}},0);
    static cfg_guid cfg_cueBackend({0xd60e2701,0x66f1,0x4a72,{0xa1,0x32,0x48,0x5e,0x1a,0x91,0x77,0x24}},GUID{});
    static cfg_guid cfg_cueDevice({0xd60e2702,0x66f1,0x4a72,{0xa1,0x32,0x48,0x5e,0x1a,0x91,0x77,0x24}},GUID{});
    static cfg_int cfg_cueVolume({0xd60e2703,0x66f1,0x4a72,{0xa1,0x32,0x48,0x5e,0x1a,0x91,0x77,0x24}},25);
    static cfg_int cfg_previewMode({0xd60e2704,0x66f1,0x4a72,{0xa1,0x32,0x48,0x5e,0x1a,0x91,0x77,0x24}},0);
    static cfg_int cfg_trimHead({0xf8071201,0x5312,0x45e9,{0x91,0x18,0x60,0x7e,0x17,0x47,0x0c,0x91}},0);
    static cfg_int cfg_trimTail({0xf8071202,0x5312,0x45e9,{0x91,0x18,0x60,0x7e,0x17,0x47,0x0c,0x91}},0);
    static cfg_int cfg_loopSnapA({0xa1b2035c,0x6135,0x4011,{0x81,0x75,0x35,0x02,0x10,0x00,0x00,0x01}},0);
    static cfg_int cfg_loopSnapB({0xa1b2035c,0x6135,0x4011,{0x81,0x75,0x35,0x02,0x10,0x00,0x00,0x02}},0);
    static cfg_int cfg_trimDb({0xf8071203,0x5312,0x45e9,{0x91,0x18,0x60,0x7e,0x17,0x47,0x0c,0x91}},-60);
    static dj_silence::Options TrimOptions(){return {cfg_trimHead.get()!=0,cfg_trimTail.get()!=0,std::clamp((int)cfg_trimDb.get(),-120,-1)};}
    static std::atomic<bool> g_english{false};
	static const wchar_t * Tr(const wchar_t * text) { return dj_language::lookup(text,g_english.load()); }

	static std::string GridKey(const std::string & path, t_uint32 subsong) {
		pfc::string8 canonical; filesystem::g_get_canonical_path(path.c_str(), canonical);
		picojson::array key; key.push_back(picojson::value(std::string(canonical.get_ptr()))); key.push_back(picojson::value((double)subsong));
		return picojson::value(key).serialize();
	}

	static constexpr UINT kRestoreMessage = WM_APP + 73;
	static constexpr unsigned kOutChannels = 2;          // le melange est toujours en stereo
	static constexpr double kHalfPi = 1.57079632679489661923;
	static constexpr unsigned kPeaksPerSecond = 100;     // precision de la forme d'onde : 100 valeurs par seconde
	static constexpr int kFaderHeight = 52;              // place reservee au crossfader, en bas de la fenetre
	static constexpr int kFooterHeight = 0;             // place reservee au numero de version, tout en bas
	static COLORREF DeckAccent(int deck) { return dj_skin::accent(deck); }
	static constexpr COLORREF kCanvas = RGB(16, 19, 25);
	static constexpr COLORREF kSurface = RGB(25, 30, 39);
	static constexpr COLORREF kText = RGB(231, 237, 245);
	static constexpr COLORREF kMuted = RGB(143, 156, 175);
	static void FillBox(CDCHandle dc, const CRect & rect, COLORREF color) {
		if (rect.Width() <= 0 || rect.Height() <= 0) return;
		CBrush brush; brush.CreateSolidBrush(color); dc.FillRect(&rect, brush);
	}
	static void BaseDrawControl(CDCHandle dc, const CRect & rect, COLORREF fill, COLORREF edge) {
		if (rect.Width() <= 0 || rect.Height() <= 0) return;
        if(rect.Width()<16 || rect.Height()<16) {
            FillBox(dc,rect,fill); CBrush border; border.CreateSolidBrush(edge); dc.FrameRect(&rect,border);
        } else dj_skin::surface(dc,rect,edge);
	}

	// Forme d'onde d'un fichier : une valeur (le pic le plus haut) toutes les 10 ms.
	// Remplie petit a petit par un thread d'analyse, lue par l'affichage.
	struct Waveform {
		std::mutex m;
		std::vector<float> peaks;
		std::vector<float> rms;
		std::shared_ptr<CImage> cover;
		bool done = false;
		abort_callback_impl abort;
	};

	// Etat d'un deck (platine).
	struct Deck {
        dj_loop::Region loop;
		std::shared_ptr<dj_stems::Reader> stems;
        unsigned sourceRate=0,sourceChannels=0;
        dj_listening::Clock listening;
		std::shared_ptr<input_helper> helper;    // le decodeur de foobar2000, ouvert sur le fichier
		std::shared_ptr<Waveform> wave;          // forme d'onde du fichier
		bool loaded = false;                     // un fichier est charge
		bool playing = false;                    // le deck est en lecture
		bool eof = false;                        // le decodeur a atteint la fin du fichier
		unsigned rate = 0, channels = 0;         // format du fichier (frequence, canaux)
		double trimStart=0,trimEnd=0;
		double length = 0;                       // duree en secondes
		std::wstring bpm;
		std::wstring musicalKey;
		double bpmValue = 0;
		std::shared_ptr<soundtouch::SoundTouch> stretch;
		bool stretchFlushed = false;

		std::vector<audio_sample> src;           // son decode, converti en stereo, a la frequence du fichier
		double srcPos = 0;                       // position de lecture (fractionnaire) dans "src", en images
		std::vector<audio_sample> pending;       // son pret a envoyer : stereo, a la frequence de la sortie
		size_t pendingPos = 0;                   // position de lecture dans "pending" (en echantillons)
		double framesPlayed = 0;               // position de lecture, en images a la frequence de la sortie

		size_t pendingFrames() const {
			return (pending.size() - pendingPos) / kOutChannels;
		}
	};

	static std::wstring FormatTime(double seconds) {
		if (seconds < 0) seconds = 0;
		const int total = (int)(seconds + 0.5);
		wchar_t buf[32];
		swprintf_s(buf, L"%d:%02d", total / 60, total % 60);
		return buf;
	}

	// Melange de deux couleurs : t = 0 -> a, t = 1 -> b.
	static COLORREF Blend(COLORREF a, COLORREF b, double t) {
		const int r = (int)(GetRValue(a) * (1.0 - t) + GetRValue(b) * t + 0.5);
		const int g = (int)(GetGValue(a) * (1.0 - t) + GetGValue(b) * t + 0.5);
		const int bl = (int)(GetBValue(a) * (1.0 - t) + GetBValue(b) * t + 0.5);
		return RGB(r, g, bl);
	}

	// Ajoute un bloc decode a d.src, converti en stereo (mono -> copie sur les deux voies,
	// plus de 2 canaux -> on garde les deux premiers).
	static void AppendStereo(Deck & d, const audio_chunk & chunk) {
		const audio_sample * p = chunk.get_data();
		const size_t frames = chunk.get_sample_count();
		const unsigned ch = d.channels;
		d.src.reserve(d.src.size() + frames * 2);
		for (size_t f = 0; f < frames; ++f) {
			const audio_sample l = p[f * ch];
			const audio_sample r = (ch >= 2) ? p[f * ch + 1] : l;
			d.src.push_back(l);
			d.src.push_back(r);
		}
	}

	// Transforme le contenu de d.src (frequence du fichier) en son pret a envoyer dans d.pending
	// (frequence de la sortie). Rereechantillonnage par interpolation lineaire.
	static void ResampleStep(Deck & d, unsigned outRate, double speed) {
		const size_t n = d.src.size() / 2;   // nombre d'images disponibles
		if (n == 0) return;

		if (d.rate == outRate && speed == 1.0 && d.srcPos == 0.0) {
			d.pending.insert(d.pending.end(), d.src.begin(), d.src.end());
			d.src.clear();
			d.srcPos = 0;
			return;
		}

		const double ratio = (double)d.rate * speed / (double)outRate;
		for (;;) {
			const size_t idx = (size_t)d.srcPos;
			if (idx + 1 >= n) break;
			const audio_sample frac = (audio_sample)(d.srcPos - (double)idx);
			d.pending.push_back(d.src[2 * idx] + (d.src[2 * idx + 2] - d.src[2 * idx]) * frac);
			d.pending.push_back(d.src[2 * idx + 1] + (d.src[2 * idx + 3] - d.src[2 * idx + 1]) * frac);
			d.srcPos += ratio;
		}
		size_t consumed = (size_t)d.srcPos;
		if (consumed > n) consumed = n;
		d.src.erase(d.src.begin(), d.src.begin() + consumed * 2);
		d.srcPos -= (double)consumed;
	}

	// Decode du son jusqu'a avoir au moins "wantFrames" images pretes (ou la fin du fichier).
	static void DrainStretch(Deck & d) {
		float samples[4096]; unsigned frames;
		while ((frames = d.stretch->receiveSamples(samples, 2048)) != 0)
			d.pending.insert(d.pending.end(), samples, samples + frames * 2);
	}
	static void FeedStretch(Deck & d, unsigned outRate) {
		const size_t start = d.pending.size();
		ResampleStep(d, outRate, 1.0); // sample-rate conversion only; SoundTouch owns speed
		if (d.pending.size() > start) {
			std::vector<float> input(d.pending.begin() + start, d.pending.end());
			d.pending.resize(start);
			d.stretch->putSamples(input.data(), (unsigned)(input.size() / 2));
		}
		DrainStretch(d);
	}
    static void ReleaseDeckReader(Deck& d) {
        d.playing=false; d.helper.reset(); d.stems.reset();
        d.src.clear(); d.srcPos=0; d.pending.clear(); d.pendingPos=0;
        d.stretch.reset(); d.stretchFlushed=false;
        // Preserve loaded state, tags, waveform and audible position for explicit resume.
    }
	static void FillDeck(Deck & d, size_t wantFrames, unsigned outRate, abort_callback & abort, double speed, bool fixedPitch, int semitones = 0) {
		if (!d.stretch) {
			d.stretch = std::make_shared<soundtouch::SoundTouch>();
			d.stretch->setSampleRate(outRate); d.stretch->setChannels(2);
			d.stretch->setPitch(1.0); d.stretchFlushed = false;
		}
		d.stretch->setPitchSemiTones(semitones);
		d.stretch->setRate(fixedPitch ? 1.0 : speed);
		d.stretch->setTempo(fixedPitch ? speed : 1.0);
		FeedStretch(d, outRate);
		while (d.pendingFrames() < wantFrames) {
			abort.check();
			if (d.eof) {
				if (!d.stretchFlushed) { d.stretch->flush(); d.stretchFlushed = true; }
				DrainStretch(d); break;
			}
			if(d.stems){if(!d.stems->read(d.src)){d.eof=true;continue;}FeedStretch(d,outRate);continue;}
            audio_chunk_impl chunk;
			if (!d.helper->run(chunk, abort)) { d.eof = true; continue; }
			if (chunk.get_channel_count() != d.channels || chunk.get_sample_rate() != d.rate) { d.eof = true; continue; }
			AppendStereo(d, chunk); FeedStretch(d, outRate);
		}
	}

	// Use foobar's front-cover lookup (embedded and external) on the analysis worker.
	static void LoadCover(const playable_location & location, const std::shared_ptr<Waveform> & wave) {
		try {
			metadb_handle_list handles; handles.add_item(metadb::get()->handle_create(location));
			pfc::list_t<GUID> ids; ids.add_item(album_art_ids::cover_front);
			const auto extractor = album_art_manager_v2::get()->open(handles, ids, wave->abort);
			const auto data = extractor->query(album_art_ids::cover_front, wave->abort);
			if (data->get_size() == 0 || data->get_size() > 32 * 1024 * 1024) return;
			HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, data->get_size());
			if (!memory) return;
			void * bytes = GlobalLock(memory);
			if (!bytes) { GlobalFree(memory); return; }
			memcpy(bytes, data->get_ptr(), data->get_size()); GlobalUnlock(memory);
			CComPtr<IStream> stream;
			if (FAILED(CreateStreamOnHGlobal(memory, TRUE, &stream))) { GlobalFree(memory); return; }
			CImage original;
			if (FAILED(original.Load(stream)) || original.IsNull() || original.GetWidth() <= 0 || original.GetHeight() <= 0) return;
			wave->abort.check();
			auto thumb = std::make_shared<CImage>();
			if (!thumb->Create(80, 80, 24)) return;
			HDC dc = thumb->GetDC();
			FillBox(CDCHandle(dc), CRect(0,0,80,80), kCanvas);
			const double scale = (std::min)(80.0 / original.GetWidth(), 80.0 / original.GetHeight());
			const int w = (std::max)(1, (int)(original.GetWidth() * scale));
			const int h = (std::max)(1, (int)(original.GetHeight() * scale));
			SetStretchBltMode(dc, HALFTONE); SetBrushOrgEx(dc, 0, 0, nullptr);
			original.Draw(dc, (80-w)/2, (80-h)/2, w, h);
			thumb->ReleaseDC();
			std::lock_guard<std::mutex> lock(wave->m); wave->cover = std::move(thumb);
		} catch (...) { /* Missing art never prevents decoding or playback. */ }
	}

	// Analyse un fichier en arriere-plan pour en tirer la forme d'onde (un pic toutes les 10 ms).
	// Utilise son propre decodeur : elle n'interfere pas avec la lecture.
	static void AnalyzeWaveform(std::string path, t_uint32 subsong, std::shared_ptr<Waveform> wave) {
		try {
			playable_location_impl location;
			location.set_path(path.c_str());
			location.set_subsong(subsong);
			LoadCover(location, wave);

			abort_callback & abort = wave->abort;
			input_helper helper;
			helper.open(service_ptr_t<file>(), location, input_flag_simpledecode, abort);

			audio_chunk_impl chunk;
			std::vector<float> local, localRms;
			double bucketEnergy = 0;
			float bucketMax = 0;
			size_t bucketFill = 0;

			while (!wave->abort.is_aborting() && helper.run(chunk, abort)) {
				const unsigned rate = chunk.get_sample_rate();
				const unsigned ch = chunk.get_channel_count();
				const size_t bucketFrames = (std::max)((size_t)1, (size_t)(rate / kPeaksPerSecond));
				const audio_sample * p = chunk.get_data();
				const size_t frames = chunk.get_sample_count();

				local.clear();
				localRms.clear();
				for (size_t f = 0; f < frames; ++f) {
					for (unsigned c = 0; c < ch; ++c) {
						const float v = std::fabs((float)p[f * ch + c]);
						if (v > bucketMax) bucketMax = v;
						bucketEnergy += (double)v * v / (std::max)(1u, ch);
					}
					if (++bucketFill >= bucketFrames) {
						local.push_back(bucketMax);
						localRms.push_back((float)std::sqrt(bucketEnergy / bucketFill));
						bucketEnergy = 0;
						bucketMax = 0;
						bucketFill = 0;
					}
				}
				if (!local.empty()) {
					std::lock_guard<std::mutex> lock(wave->m);
					wave->peaks.insert(wave->peaks.end(), local.begin(), local.end());
					wave->rms.insert(wave->rms.end(), localRms.begin(), localRms.end());
				}
			}
			if (bucketFill > 0 && !wave->abort.is_aborting()) {
				std::lock_guard<std::mutex> lock(wave->m);
				wave->peaks.push_back(bucketMax);
				wave->rms.push_back((float)std::sqrt(bucketEnergy / bucketFill));
			}
		} catch (...) {
			// Pas de forme d'onde pour ce fichier : tant pis, la lecture n'en depend pas.
		}
		std::lock_guard<std::mutex> lock(wave->m);
		wave->done = true;
	}

    struct PreparedTrack{Deck deck;file_info_impl info;std::string error;};
    static std::shared_ptr<PreparedTrack> PrepareTrack(const dj_queue::Item& item,dj_silence::Options options,abort_callback_impl& abort){
        auto ready=std::make_shared<PreparedTrack>();auto& d=ready->deck;
        d.helper=std::make_shared<input_helper>();
        d.helper->open(service_ptr_t<file>(),make_playable_location(item.path.c_str(),item.subsong),input_flag_no_looping,abort);
        d.helper->get_info(item.subsong,ready->info,abort);d.length=ready->info.get_length();
        auto bounds=dj_silence::analyze<audio_chunk_impl>(*d.helper,d.length,options,abort);
        d.trimStart=bounds.start;d.trimEnd=bounds.end;
        if(d.helper->can_seek())d.helper->seek(d.trimStart,abort);
        // Decode a bounded quarter-second of source audio now, not at transition.
        for(unsigned n=0;n<64;++n){abort.check();audio_chunk_impl chunk;if(!d.helper->run(chunk,abort)){d.eof=true;break;}
            if(!chunk.get_sample_count())continue;
            if(!d.rate){d.rate=chunk.get_sample_rate();d.channels=chunk.get_channel_count();d.sourceRate=d.rate;d.sourceChannels=d.channels;}
            if(!d.rate||!d.channels||d.channels>256||d.rate!=chunk.get_sample_rate()||d.channels!=chunk.get_channel_count())throw std::runtime_error("Preparation: unsupported audio format");
            AppendStereo(d,chunk);if(d.src.size()/2>=(std::min)(size_t(128000),size_t(d.rate/4)))break;
        }
        if(d.src.empty())throw std::runtime_error("No decoded audio during preparation");return ready;
    }

	// Position de toutes les zones d'un deck dans la fenetre.
	struct DeckLayout {
		CRect deck;       // cadre du deck
		CRect cover;
		CRect title;      // "DECK A"
		CRect gridControl[4];
        CRect loop[4],loopSnap;
        CRect pitch[3];
        CRect matchKey;
        CRect analyse[3];
        CRect release;
        CRect automation,headphones;
        CRect stemPrepare,stem[4],stemSolo[4],stemVolume[4];
        CRect queue[2],queueOpen;
		CRect beat;
		CRect key;
		CRect sync;
		CRect speed;
        CRect range;
		CRect artist;
		CRect name;       // nom du fichier
		CRect info;       // temps ecoule / duree / format
		CRect zoomOut;    // bouton "-"
		CRect zoomIn;     // bouton "+"
		CRect wave;       // forme d'onde
		CRect vol;        // curseur de volume (colonne a droite de la forme d'onde)
		CRect btn[2];     // Lecture-Pause / Debut
	};

	static DeckLayout MakeLayout(int deck, const CRect & client) {
		DeckLayout L;
		const int margin = 10;
		const int half = client.Width() / 2;
		L.deck = CRect(client.left + deck * half + margin,
		               client.top + margin,
		               client.left + (deck + 1) * half - margin,
		               client.bottom - margin - kFaderHeight - kFooterHeight);
		const CRect & d = L.deck;
		const int left = d.left + 8, right = d.right - 8;

		L.cover = CRect(left, d.top + 8, left + 80, d.top + 88);
		const int headerLeft = left + 90;
		L.title = CRect(headerLeft, d.top + 8, right, d.top + 30);
		L.name = CRect(headerLeft, d.top + 32, right, d.top + 54);
		L.artist = CRect(headerLeft, d.top + 56, right, d.top + 78);
		L.info = CRect(left, d.top + 92, right - 52, d.top + 114);
		L.zoomOut = CRect(right - 48, d.top + 92, right - 26, d.top + 114);
		L.zoomIn = CRect(right - 24, d.top + 92, right, d.top + 114);

        // Compact, raised hardware-like controls; the waveform keeps most of the space.
        const int controlWidth=(std::min)(64,(right-left-5*8)/6);
        auto control=[&](int n){int x=left+n*(controlWidth+8);return CRect(x,d.top+122,x+controlWidth,d.top+164);};
        for(int k=0;k<3;++k)L.analyse[k]=control(k);
        L.release=control(3);L.headphones=control(4);L.automation=control(5);L.automation.right=right;
        for(int n=0;n<2;++n)L.queue[n]=CRect(left+4,d.top+170+n*22,right-42,d.top+192+n*22);
        L.queueOpen=CRect(right-38,d.top+172,right,d.top+212);
        L.stemPrepare=CRect(left,d.top+220,left+90,d.top+252);
        int stemWidth=(right-left-94)/4;
        for(int n=0;n<4;++n){
            int x=left+94+n*stemWidth,end=left+94+(n+1)*stemWidth-3;
            L.stem[n]=CRect(x,d.top+220,end,d.top+244);
            L.stemSolo[n]=CRect(x,d.top+246,x+22,d.top+270);
            L.stemVolume[n]=CRect(x+24,d.top+246,end,d.top+270);
        }
        const int waveTop=d.top+278,waveBottom=d.bottom-196;
        L.wave=CRect(left,waveTop,right-46,waveBottom);
        L.vol=CRect(right-38,waveTop,right,waveBottom);
        for(int k=0;k<4;++k){int w=(right-left-40)/4;L.loop[k]=CRect(left+k*w,d.bottom-190,left+(k+1)*w-4,d.bottom-160);}
        L.loopSnap=CRect(right-36,d.bottom-190,right,d.bottom-160);
        const int row=d.bottom-154;
        for(int k=0;k<4;++k) L.gridControl[k]=CRect(left+k*36,row,left+k*36+32,row+36);
        L.pitch[0]=CRect(left+148,row,left+176,row+36);
        L.pitch[1]=CRect(left+180,row,right-124,row+36);
        L.pitch[2]=CRect(right-120,row,right-92,row+36);
        L.matchKey=CRect(right-84,row,right-48,row+36);
        L.key=CRect(right-40,row,right,row+36);
        L.speed=CRect(left,d.bottom-108,right,d.bottom-56);
        L.range=CRect(left,L.speed.top,left+64,L.speed.top+32);
        L.beat=CRect(right-76,L.speed.top,right-40,L.speed.top+32);
        L.sync=CRect(right-36,L.speed.top,right,L.speed.top+32);
        const int gap=8, w=(right-left-gap)/2;
        for(int k=0;k<2;++k) L.btn[k]=CRect(left+k*(w+gap),d.bottom-48,left+k*(w+gap)+w,d.bottom-6);

		return L;
	}

	// Bande du crossfader : tout en bas, sous les deux decks.
    static CRect HeaderStatusRect(const DeckLayout& L,int textWidth){
        CRect status=L.title;status.right-=12;status.left=(std::max)(L.title.left+62,status.right-textWidth-8);return status;
    }
	static CRect FaderStrip(const CRect & client) {
		const int margin = 10;
		return CRect(client.left + margin, client.bottom - margin - kFaderHeight - kFooterHeight + 6, client.right - margin, client.bottom - margin - kFooterHeight);
	}
    static CRect PlayBothRect(const CRect& client) {
        auto strip=FaderStrip(client);return CRect(strip.left+8,strip.top+6,strip.left+66,strip.bottom-6);
    }
    static int FaderLeft(const CRect& client) { return FaderStrip(client).left+90; }
    static int FaderRight(const CRect& client) { return FaderStrip(client).right-164; }
    static CRect MasterMeterRect(const CRect& client){auto strip=FaderStrip(client);return CRect(strip.right-138,strip.top+6,strip.right-70,strip.bottom-6);}
    static CRect VersionRect(const CRect&) { return CRect(0,0,0,0); }
    static CRect LanguageRect(const CRect& client,bool second) {
        if(second) return CRect(0,0,0,0);
        auto strip=FaderStrip(client);return CRect(strip.right-64,strip.top+6,strip.right-8,strip.bottom-6);
    }

	// Colonne du curseur de volume : zone utile de la glissiere verticale.
	static int VolTrackTop(const DeckLayout & L) { return L.vol.top + 22; }
	static int VolTrackBottom(const DeckLayout & L) { return L.vol.bottom - 6; }

	class CDjDropTarget;   // defini plus bas : recoit les morceaux glisses sur le panneau

	class CDjPanel : public ui_element_instance, public CWindowImpl<CDjPanel>, public metadb_io_callback_dynamic_impl_base, public completion_notify_receiver {
	public:
		// Nom interne de la classe de fenetre (identifiant unique, different de celui de l'exemple).
		DECLARE_WND_CLASS_EX(TEXT("{3E9B6D14-2A7C-4F58-B1D6-8A4C05E7D932}"), CS_VREDRAW | CS_HREDRAW, (-1));

		void initialize_window(HWND parent) {
			m_outputRatePreference=cfg_outputRate.get();m_limitEnabled=cfg_masterLimiter.get()!=0;m_limitCeiling=std::clamp(double(cfg_masterCeiling.get()),-120.,-1.)/10.;
            WIN32_OP(Create(parent) != NULL);
			SetTimer(kTimerId, 33);    // Cap UI animation at 30 fps; audio clocks are independent.
			MidiInitialize();
            RegisterDrop();            // le panneau accepte les morceaux glisses dessus
			PostMessage(kRestoreMessage); // one restoration after window creation, no watcher
		}

		BEGIN_MSG_MAP_EX(CDjPanel)
			MESSAGE_HANDLER(kRestoreMessage, OnRestoreSession)
            MESSAGE_HANDLER(WM_NOTIFY, OnTooltipNotify)
            MESSAGE_HANDLER(WM_APP + 173, OnMetadataRefresh)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_RBUTTONDOWN(OnRButtonDown)
			MSG_WM_MOUSEMOVE(OnMouseMove)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
            MSG_WM_CAPTURECHANGED(OnCaptureChanged)
			MSG_WM_MOUSEWHEEL(OnMouseWheel)
			MSG_WM_ERASEBKGND(OnEraseBkgnd)
			MSG_WM_PAINT(OnPaint)
			MSG_WM_TIMER(OnTimer)
			MSG_WM_DESTROY(OnDestroy)
		END_MSG_MAP()

		CDjPanel(ui_element_config::ptr config, ui_element_instance_callback_ptr p_callback);
		HWND get_wnd() { return *this; }
		void set_configuration(ui_element_config::ptr config) { m_config = config; }
		ui_element_config::ptr get_configuration() { return m_config; }
		static GUID g_get_guid() { return guid_dj_panel; }
		static GUID g_get_subclass() { return ui_element_subclass_utility; }
		static void g_get_name(pfc::string_base & out) { out = "DJ Decks"; }
		static ui_element_config::ptr g_get_default_configuration() { return ui_element_config::g_create_empty(g_get_guid()); }
		static const char * g_get_description() { return g_english.load() ? "Dual DJ decks (Deck A / Deck B)." : "Double platine DJ (Deck A / Deck B)."; }

		void notify(const GUID & p_what, t_size p_param1, const void * p_param2, t_size p_param2size);

		// Appeles par la cible de glisser-deposer (CDjDropTarget), pas par le reste du code.
		HRESULT DropEnter(IDataObject * obj, POINTL pt, DWORD * effect);
		HRESULT DropOver(POINTL pt, DWORD * effect);
		HRESULT DropLeave();
		HRESULT DropDrop(IDataObject * obj, POINTL pt, DWORD * effect);

    public:
        void on_changed_sorted(metadb_handle_list_cref items, bool) override {
            if(!m_hWnd)return;
            for(int i=0;i<2;++i){
                for(t_size n=0;n<m_queueHandles[i].get_count();++n)if(metadb_handle_list_helper::bsearch_by_pointer(items,m_queueHandles[i][n])!=SIZE_MAX){PostMessage(WM_APP+173);return;}
                if(!m_savedPath[i].empty()){auto h=metadb::get()->handle_create(m_savedPath[i].c_str(),m_savedSubsong[i]);if(metadb_handle_list_helper::bsearch_by_pointer(items,h)!=SIZE_MAX){PostMessage(WM_APP+173);return;}}
            }
        }
        void on_task_completion(unsigned id,unsigned status) override {
            orphan_task(id);if(status!=metadb_io::load_info_success)console::print("DJ Decks: some playlist metadata could not be read; use Reload playlist metadata to retry.");
            if(m_hWnd)PostMessage(WM_APP+173);
        }
    private:
        LRESULT OnMetadataRefresh(UINT,WPARAM,LPARAM,BOOL&) { RefreshDeckTags(0); RefreshDeckTags(1); UpdateQueue(0,m_queueWindow[0].list?m_queueWindow[0].selected():std::vector<size_t>{}); UpdateQueue(1,m_queueWindow[1].list?m_queueWindow[1].selected():std::vector<size_t>{}); return 0; }
        dj_midi::Input m_midiInput;dj_midi::Router m_midiRouter;std::vector<dj_midi::Device> m_midiDevices;
        std::vector<dj_midi::Profile> m_midiProfiles;int m_midiAction=0;std::wstring m_midiStatus;
        void MidiInitialize();void MidiConfigure();void MidiRefreshDevices();void MidiSave();void MidiTick();
        double MidiCurrent(int action);void MidiApply(int action,double value);
        std::vector<dj_settings::Item> MidiValues();void MidiSetting(int id,int value);
        void RefreshDeckTags(int deck, bool readFile = false);
        void ReleaseForTags(int deck);
        bool ReopenAfterTags(int deck);
        void AnalyseDeck(int deck,int action);
        void MatchKey(int deck);
		static constexpr UINT_PTR kTimerId = 1;
		static constexpr size_t kBlockFrames = 1024;   // taille d'un bloc de son envoye a la sortie

		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnLButtonUp(UINT nFlags, CPoint point);
		void OnRButtonDown(UINT nFlags, CPoint point);
		void OnMouseMove(UINT nFlags, CPoint point);
        CRect m_hotButton={0,0,0,0},m_pressedButton={0,0,0,0};
        bool m_trackingMouse=false;
        CRect HitButton(CPoint point,const CRect& rc) const {
            for(auto r:{PlayBothRect(rc),LanguageRect(rc,false)}) if(r.PtInRect(point))return r;
            for(int i=0;i<2;++i) {
                auto L=MakeLayout(i,rc);
                for(auto r:{L.btn[0],L.btn[1],L.analyse[0],L.analyse[1],L.analyse[2],L.release,L.headphones,L.automation,L.stemPrepare,L.stem[0],L.stem[1],L.stem[2],L.stem[3],L.stemSolo[0],L.stemSolo[1],L.stemSolo[2],L.stemSolo[3],L.queueOpen,L.zoomOut,L.zoomIn,
                    L.loopSnap,L.loop[0],L.loop[1],L.loop[2],L.loop[3],L.gridControl[0],L.gridControl[1],L.gridControl[2],L.gridControl[3],L.pitch[0],L.pitch[1],L.pitch[2],L.matchKey,L.key,L.beat,L.sync,L.range})
                    if(r.PtInRect(point))return r;
            }
            return CRect(0,0,0,0);
        }
        void DrawControl(CDCHandle dc,const CRect& r,COLORREF fill,COLORREF edge) {
            if(!r.IsRectEmpty() && (r==m_hotButton || r==m_pressedButton))
                dj_skin::surface(dc,r,edge,false,true,r==m_pressedButton);
            else BaseDrawControl(dc,r,fill,edge);
        }
        void InvalidateButton(const CRect& rect){if(rect.IsRectEmpty())return;auto dirty=dj_tooltip::feedback(rect);InvalidateRect(&dirty,FALSE);}
        void OnMouseLeave() { m_trackingMouse=false;auto old=m_hotButton;m_hotButton.SetRectEmpty();InvalidateButton(old); }
        void OnCaptureChanged(HWND) {auto old=m_pressedButton;bool dragging=m_drag!=0;m_pressedButton.SetRectEmpty();m_drag=0;if(dragging)Invalidate();else InvalidateButton(old);}
		BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		void OnPaint(CDCHandle);
        CBitmap m_paintBuffer;int m_paintWidth=0,m_paintHeight=0;
        struct Tip { CRect rect; std::wstring text; };
        HWND m_tooltip=nullptr;
        CRect m_tipClient={0,0,0,0};
        bool m_tipEnglish=false;
        std::vector<Tip> m_tips;
        void UpdateTooltips(const CRect& client);
        LRESULT OnTooltipNotify(UINT,WPARAM,LPARAM value,BOOL& handled) {
            auto hdr=reinterpret_cast<NMHDR*>(value);
            if(hdr && hdr->hwndFrom==m_tooltip && hdr->code==TTN_GETDISPINFOW) {
                auto info=reinterpret_cast<NMTTDISPINFOW*>(value);
                if(hdr->idFrom>0 && hdr->idFrom<=m_tips.size()) info->lpszText=const_cast<wchar_t*>(m_tips[hdr->idFrom-1].text.c_str());
                return 0;
            }
            handled=FALSE; return 0;
        }
		void PaintAll(CDCHandle dc, const CRect & rc);
		BOOL OnEraseBkgnd(CDCHandle);
		// On ne redessine vite que si quelque chose bouge (lecture, curseur deplace) ; au repos, 4 fois par seconde.
		void OnTimer(UINT_PTR id) {
            MidiTick();PreparationTick();PreviewTick();ScrobbleTick();StemsTick();AutomationTick();
			if (m_cachePlaying[0] || m_cachePlaying[1] || m_drag != 0 || m_dragDirty) { m_dragDirty=false;Invalidate(FALSE); return; }
			CRect client; GetClientRect(&client);
			for (int i = 0; i < 2; ++i) {
				const auto layout = MakeLayout(i, client);
				if (m_scrollActive[i][0]) InvalidateRect(&layout.name, FALSE);
				if (m_scrollActive[i][1]) InvalidateRect(&layout.artist, FALSE);
			}
			if ((++m_idleTicks % 8) == 0) Invalidate();
		}
		void OnDestroy();

		void LoadDeck(int deck, const char * path, t_uint32 subsong);
		void SaveSession();
        dj_session::Bundle CaptureBundle(bool full);
        void ApplyBundle(const dj_session::Bundle& bundle,bool restoreDecks);
        void DistributeActivePlaylist();
        void ExportBundle(bool full);void ImportBundle(bool recovery=false);
        bool m_applyImport[2]={};dj_session::Deck m_importDeck[2];
        std::unique_ptr<dj_stems::Job> m_stemJob;
        std::shared_ptr<abort_callback_impl> m_stemAbort;
        int m_stemOwner=-1;
        std::string m_stemKey[2];
        bool m_stemReady[2]={},m_stemActive[2]={};
        unsigned m_stemMask[2]={15,15},m_stemSolo[2]={};
        float m_stemVolume[2][4]={{1,1,1,1},{1,1,1,1}};
        void SetStemVolume(int deck,int stem,float value);
        void SetStemVolumeFromX(int deck,int stem,int x,const CRect& client);
        void SoloStem(int deck,int stem);
        dj_cue::Sink m_cueOutput;
        std::atomic<bool> m_cue[2]={{false},{false}};
        std::atomic<double> m_cueVolume{0.25};
        struct PreviewRequest{dj_queue::Item item;int mode=0;size_t token=0;double position=-1;};
        std::atomic<bool> m_previewActive{false};bool m_previewOwned=false;
        int m_previewDeck=-1;dj_queue::Item m_previewItem;
        dj_preview::Visual m_previewVisual;DjPreviewWindow m_previewWindow;ULONGLONG m_previewPaint=0;
        void PlayPreview(const PreviewRequest& request,abort_callback_impl& abort);
        std::string PreviewOverviewKey(const dj_queue::Item& item);
        void ScanPreviewOverview(const PreviewRequest& request,abort_callback_impl& abort);
        dj_preview::OverviewCache m_previewOverviewCache;
        dj_preview::Runner<PreviewRequest,abort_callback_impl> m_previewOverview{[this](const PreviewRequest& r,abort_callback_impl& a){ScanPreviewOverview(r,a);}};
        dj_preview::Runner<PreviewRequest,abort_callback_impl> m_previewPlayer{[this](const PreviewRequest& r,abort_callback_impl& a){PlayPreview(r,a);}};
        void ConfigureCue(){m_cueOutput.configure((m_previewActive.load()||m_cue[0].load()||m_cue[1].load())&&cfg_cueBackend.get()!=GUID{},cfg_cueBackend.get(),cfg_cueDevice.get());}
        void StopPreview(bool wait=false);
        void StartPreview(int deck,size_t row);
        void SeekPreview(double seconds);
        void PreviewTick();
        void CueMenu(CPoint point);
        DjSettingsWindow m_settings;
        struct SettingsDevice{std::wstring name;GUID backend,device;};
        std::vector<SettingsDevice> m_settingsDevices;
        std::wstring m_settingsDeviceError;
        void OpenSettings(int page=-1);
        void RefreshSettingsDevices();
        std::vector<dj_settings::Item> SettingsValues(int page);
        void SettingsChanged(int id,int value);
        void SettingsCommand(int choice);
        void RefreshTrimPreparations();
        struct PrepareRequest{int deck=0;dj_queue::Item item;dj_silence::Options options;std::string key;};
        std::mutex m_prepareMutex;std::string m_prepareKey[2];std::shared_ptr<PreparedTrack> m_prepared[2];
        bool m_pendingLoad[2]={},m_pendingPlay[2]={},m_prepareSuspended[2]={};double m_pendingSeek[2]={-1,-1};dj_queue::Item m_pendingItem[2];
        void Prepare(const PrepareRequest& r,abort_callback_impl& abort){
            ::SetThreadPriority(::GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);std::shared_ptr<PreparedTrack> result;
            try{result=PrepareTrack(r.item,r.options,abort);}catch(const std::exception& e){if(abort.is_aborting())return;result=std::make_shared<PreparedTrack>();result->error=e.what();}
            catch(...){if(abort.is_aborting())return;result=std::make_shared<PreparedTrack>();result->error="Unknown track preparation error";}
            std::lock_guard<std::mutex> lock(m_prepareMutex);if(!abort.is_aborting()&&m_prepareKey[r.deck]==r.key)m_prepared[r.deck]=result;
        }
        dj_preview::Runner<PrepareRequest,abort_callback_impl> m_prepareA{[this](const PrepareRequest& r,abort_callback_impl& a){Prepare(r,a);}};
        dj_preview::Runner<PrepareRequest,abort_callback_impl> m_prepareB{[this](const PrepareRequest& r,abort_callback_impl& a){Prepare(r,a);}};
        static std::string PrepareKey(const dj_queue::Item& item){return item.path+"\n"+std::to_string(item.subsong)+"\n"+TrimOptions().key();}
        void RequestPreparation(int deck,const dj_queue::Item& item){
            auto key=PrepareKey(item);{std::lock_guard<std::mutex> lock(m_prepareMutex);if(m_prepareKey[deck]==key)return;m_prepareKey[deck]=key;m_prepared[deck].reset();}
            PrepareRequest request{deck,item,TrimOptions(),key};if(deck)m_prepareB.start(request);else m_prepareA.start(request);
        }
        std::shared_ptr<PreparedTrack> TakePreparation(int deck,const dj_queue::Item& item){
            std::lock_guard<std::mutex> lock(m_prepareMutex);if(m_prepareKey[deck]!=PrepareKey(item))return {};
            auto result=std::move(m_prepared[deck]);if(result)m_prepareKey[deck].clear();return result;
        }
        void DropPreparation(int deck){
            std::shared_ptr<PreparedTrack> released;
            {std::lock_guard<std::mutex> lock(m_prepareMutex);if(m_prepareKey[deck].empty())return;m_prepareKey[deck].clear();released=std::move(m_prepared[deck]);}
            if(deck)m_prepareB.cancel();else m_prepareA.cancel();
        }
        bool NextPrepared(int deck){
            if(m_prepareSuspended[deck])return false;if(!TrimOptions().enabled())return true;int next=m_queueCurrent[deck]+1;if(next<0||next>=(int)m_queue[deck].size())return true;
            RequestPreparation(deck,m_queue[deck][next]);std::lock_guard<std::mutex> lock(m_prepareMutex);return bool(m_prepared[deck]);
        }
        void ClearPreparations(bool stop=false){
            if(stop){m_prepareA.stop();m_prepareB.stop();}else{m_prepareA.cancel(true);m_prepareB.cancel(true);}
            std::lock_guard<std::mutex> lock(m_prepareMutex);for(int i=0;i<2;++i){m_prepared[i].reset();m_prepareKey[i].clear();}
        }
        void PreparationTick();void SilenceMenu(CPoint point);
        void CommitLoadDeck(int deck,const char* path,t_uint32 subsong,std::shared_ptr<PreparedTrack> prepared={});
        void CheckStems(int deck);
        void StartStems(int deck);
        void StemsMenu(int deck,CPoint point);
        void ToggleStem(int deck,int stem);
        void UseStems(int deck,unsigned mask);
        void OriginalAudio(int deck);
        void CancelStems();
        std::atomic<double> m_rgTrack[2]={{1},{1}},m_rgAlbum[2]={{1},{1}};
        ULONGLONG m_lastScrobbleTick=0; std::atomic<bool> m_scrobbleAllowed[2]={{false},{false}};
        void UpdateReplayGain(int deck,const file_info& info){
            t_replaygain_config cfg;cfg.m_processing_mode=t_replaygain_config::processing_mode_gain_and_peak;
            cfg.m_source_mode=t_replaygain_config::source_mode_track;m_rgTrack[deck]=dj_listening::safeScale(cfg.query_scale(info));
            cfg.m_source_mode=t_replaygain_config::source_mode_album;m_rgAlbum[deck]=dj_listening::safeScale(cfg.query_scale(info));
        }
        void ScrobbleTick(){
            auto now=GetTickCount64();if(now-m_lastScrobbleTick<1000)return;m_lastScrobbleTick=now;
            try{
                auto api=g_scrobble.load()?dj_scrobble::service():foo_scrobble::ScrobbleService::ptr{};
                bool active=g_scrobble.load()&&api.is_valid()&&dj_scrobble::hostEnabled();g_scrobbleReady=active;
                metadb_handle_ptr normal;playback_control::get()->get_now_playing(normal);
                for(int i=0;i<2;++i){double duration=0;long long start=0;
                    metadb_handle_ptr h;if(!m_savedPath[i].empty())h=metadb::get()->handle_create(m_savedPath[i].c_str(),m_savedSubsong[i]);
                    bool allowed=active&&h.is_valid()&&(!normal.is_valid()||normal!=h);m_scrobbleAllowed[i]=allowed;
                    {std::lock_guard<std::mutex> lock(m_lock);auto& d=m_deck[i];if(!allowed){d.listening.discardUnsubmitted();continue;}if(!d.listening.eligible(d.length))continue;duration=d.length;start=d.listening.started;}
                    file_info_impl info;if(!h->get_info(info))continue;
                    auto item=dj_scrobble::track(info,duration,start);if(item.Artist.is_empty()||item.Title.is_empty())continue;
                    // Do not duplicate the track already managed by normal foobar playback.

                    {std::lock_guard<std::mutex> lock(m_lock);m_deck[i].listening.submitted=true;}
                    api->ScrobbleAsync(std::move(item));
                    console::print("DJ Decks: qualified deck listening queued through foo_scrobble.");
                }
            }catch(const std::exception& e){g_scrobbleReady=false;for(auto& allowed:m_scrobbleAllowed)allowed=false;console::print((std::string("DJ Decks: foo_scrobble bridge: ")+e.what()).c_str());}
        }

        void StemsTick();
        bool m_chain[2]={false,false},m_alternate=false,m_autoChanging=false,m_autoArmed[2]={false,false};
        bool m_altAvailable[2]={true,true};
        int m_autoActive=0;
        double m_fadeSeconds=5;
        std::atomic<bool> m_linearFade{false};
        dj_auto::Fade m_autoFade; // guarded by m_lock, advanced only by audio output frames
        void AutomationMenu(int deck,CPoint point);
        void AutomationTick();
        void CancelAutomation();
        bool NextQueue(int deck,bool play);
        std::vector<dj_queue::Item> m_queue[2];
        std::vector<std::wstring> m_queueLabels[2];
        metadb_handle_list m_queueHandles[2],m_dropHandles;
        std::set<std::string> m_queueInfoAttempted;bool m_forceQueueInfo=false;
        void EnsureQueueInfo(){
            // Keep all old references alive until BOTH new lists have been built:
            // moving the last occurrence A -> B must not evict its cached metadata.
            metadb_handle_list retained[2];
            for(int i=0;i<2;++i)for(auto& item:m_queue[i])retained[i].add_item(metadb::get()->handle_create(item.path.c_str(),item.subsong));
            m_queueHandles[0]=retained[0];m_queueHandles[1]=retained[1];
            std::set<std::string> live;for(int i=0;i<2;++i)for(auto& item:m_queue[i])live.insert(GridKey(item.path,item.subsong));
            for(auto it=m_queueInfoAttempted.begin();it!=m_queueInfoAttempted.end();)if(!live.count(*it))it=m_queueInfoAttempted.erase(it);else ++it;
            if(have_task(1))return;
            metadb_handle_list missing;bool force=m_forceQueueInfo;m_forceQueueInfo=false;
            for(int i=0;i<2;++i)for(t_size n=0;n<retained[i].get_count();++n){
                auto h=retained[i][n];file_info_impl info;
                bool have=h->get_info(info);auto title=have?info.meta_get("TITLE",0):nullptr;auto artist=have?info.meta_get("ARTIST",0):nullptr;
                bool need=force||!have||!std::isfinite(h->get_length())||h->get_length()<=0||!title||!*title||!artist||!*artist;
                if(!need)continue;
                auto native=filesystem::g_get_native_path(h->get_path());if(native.is_empty())continue;
                auto key=GridKey(h->get_path(),h->get_subsong_index());if(!m_queueInfoAttempted.insert(key).second)continue;
                missing.add_item(h);
            }
            if(!missing.get_count())return;
            try{metadb_io_v2::get()->load_info_async(missing,metadb_io::load_info_force,nullptr,metadb_io_v2::op_flag_silent|metadb_io_v2::op_flag_no_errors,create_task(1));}
            catch(const std::exception& e){orphan_task(1);console::print((std::string("DJ Decks: playlist metadata: ")+e.what()).c_str());}
        }

        int m_queueCurrent[2]={-1,-1};
        DjQueueWindow m_queueWindow[2];
        void OpenQueue(int deck);
        void UpdateQueue(int deck,const std::vector<size_t>& selected={});
        void QueueAction(int deck,int action,std::vector<size_t> selected,size_t before);
        void AddQueue(int deck,IDataObject* object,size_t before);
        std::vector<dj_queue::Item> ExtractItems(IDataObject* object);
        std::wstring QueueLabel(const dj_queue::Item& item);
        double QueueDuration(const dj_queue::Item& item,metadb_handle_ptr handle){
            double seconds=handle->get_length();if(std::isfinite(seconds)&&seconds>0)return seconds;
            std::lock_guard<std::mutex> lock(m_lock);for(int i=0;i<2;++i)if(m_savedPath[i]==item.path&&m_savedSubsong[i]==item.subsong&&m_deck[i].loaded)return m_deck[i].length;
            return 0;
        }
        titleformat_object::ptr m_queueFormat;std::string m_queueFormatText;
        void QueueFormatMenu(int choice){
            if(choice==305){cfg_queueNumbers=cfg_queueNumbers.get()?0:1;}
            else if(choice>=300&&choice<304){cfg_queueFormat=dj_queue_format::presets[choice-300];}
            else if(choice==304){
                std::wstring script=pfc::stringcvt::string_wide_from_utf8(cfg_queueFormat.get().get_ptr()).get_ptr();
                auto validate=[this](const std::wstring& input,std::wstring& preview){
                    titleformat_object::ptr format;std::string text=pfc::stringcvt::string_utf8_from_wide(input.c_str()).get_ptr();
                    if(input.empty()||input.size()>4096||!titleformat_compiler::get()->compile(format,text.c_str())){preview=g_english.load()?L"Invalid title-format expression.":L"Expression de format invalide.";return false;}
                    preview=g_english.load()?L"Valid expression. Preview: ":L"Expression valide. Aperçu : ";
                    for(int deck=0;deck<2;++deck)if(!m_queue[deck].empty()){auto& item=m_queue[deck][0];auto h=metadb::get()->handle_create(item.path.c_str(),item.subsong);pfc::string8 result;DjQueueDurationHook duration(QueueDuration(item,h));h->format_title(&duration,result,format,nullptr);preview+=dj_queue_format::singleLine(pfc::stringcvt::string_wide_from_utf8(result).get_ptr());return true;}
                    preview+=g_english.load()?L"load a playlist to preview.":L"charger une playlist pour l'aperçu.";return true;
                };
                if(!dj_queue_format::edit(m_hWnd,g_english.load(),script,validate))return;
                cfg_queueFormat=pfc::stringcvt::string_utf8_from_wide(script.c_str()).get_ptr();
            }
            m_queueFormat.release();m_queueFormatText.clear();
            for(int i=0;i<2;++i)UpdateQueue(i,m_queueWindow[i].list?m_queueWindow[i].selected():std::vector<size_t>{});
        }
        bool TransferQueue(int from,int to,const std::vector<size_t>& selection,size_t before,bool moving){
            std::vector<size_t> inserted;
            if(!dj_queue::transfer(m_queue[from],m_queueCurrent[from],m_queue[to],m_queueCurrent[to],selection,before,moving,inserted)){
                ::MessageBoxW(m_hWnd,g_english.load()?L"Cannot transfer: invalid selection or playlist storage limit (10000 tracks, 3 MiB).":L"Transfert impossible : sélection invalide ou limite des listes (10000 pistes, 3 Mio).",L"DJ Decks",MB_OK|MB_ICONINFORMATION);return false;
            }
            if(m_savedPath[to].empty()&&!inserted.empty()){m_queueCurrent[to]=(int)inserted.front();auto item=m_queue[to][inserted.front()];LoadDeck(to,item.path.c_str(),item.subsong);}
            SaveSession();UpdateQueue(from,moving?std::vector<size_t>{}:selection);UpdateQueue(to,inserted);return true;
        }

		LRESULT OnRestoreSession(UINT, WPARAM, LPARAM, BOOL &);
		std::string m_savedPath[2];
		t_uint32 m_savedSubsong[2] = {0, 0};
		bool m_restoring = false;

		// Glisser-deposer
		void RegisterDrop();
		void RevokeDrop();
		int DeckAtScreenPoint(POINTL pt);
		bool CanAccept(IDataObject * obj);
		bool ExtractFirstItem(IDataObject * obj, pfc::string8 & path, t_uint32 & subsong);
		void TogglePlay(int deck);
		void PlayBoth();
		void SeekDeck(int deck, double seconds);
        void EditLoop(int deck,int action);
        void SetLoopSnap(int deck,bool enabled);
        bool SnapLoopPoint(int deck,double position,double lower,double upper,double& target);
        bool m_loopSnap[2]={cfg_loopSnapA.get()!=0,cfg_loopSnapB.get()!=0};
        dj_loop::Region m_cacheLoop[2];
        std::wstring m_loopNotice[2];
        picojson::object m_loopLibrary;
        void SaveLoopPoints(int deck,const dj_loop::Region& points);
		void EnsureEngine();
		void StopEngine();
		void StopWaveThread(int deck);
		void EngineThread();

		// Interface : curseurs, zoom, forme d'onde
		void SetFaderFromX(int x, const CRect & client);
		void SetVolumeFromY(int deck, int y, const CRect & client);
		void SetSpeedFromX(int deck, int x, const CRect & client);
		void MatchTempo(int deck);
		void SyncBeats(int deck);
		void EditGrid(int deck, int action, double step = 0.01);
		void ZoomDeck(int deck, bool zoomIn);
		void ViewWindow(int deck, double & t0, double & width) const;
		void SeekFromWaveX(int deck, int x, const CRect & client);
		void PaintWave(CDCHandle dc, const DeckLayout & L, int deck, COLORREF textColor, COLORREF bgColor);

		ui_element_config::ptr m_config;

		// --- Reserve au thread de l'interface ---
		std::wstring m_artist[2];
		std::wstring m_name[2];                // nom du fichier (affiche a l'ecran)
		int m_drag = 0;                        // curseur en cours de deplacement : 0 aucun, 1 crossfader, 2 volume A, 3 volume B
		double m_view[2] = { 0.0, 0.0 };       // duree visible de la forme d'onde, en secondes (0 = tout le morceau)
		std::thread m_waveThread[2];           // threads d'analyse des formes d'onde
		std::shared_ptr<Waveform> m_waveCur[2];

		// Dernieres valeurs lues dans les decks, pour l'affichage (thread de l'interface seulement)
		std::wstring m_cacheInfo[2];
        std::wstring m_cacheKey[2];
		bool m_cachePlaying[2] = { false, false };
		bool m_cacheLoaded[2] = { false, false };
        bool m_cacheReleased[2] = {};
        bool m_waveNeedsRebuild[2] = {};
		double m_cacheLength[2] = { 0.0, 0.0 };
		double m_cacheBpm[2] = {};
		dj_beats::Grid m_grid[2];
		bool m_manualGrid[2] = {};
		double m_manualPhase[2] = {};
		picojson::object m_gridLibrary;
		std::shared_ptr<Waveform> m_gridSource[2];
		ULONGLONG m_gridStamp[2] = {};
		double m_gridPosition[2] = {}, m_gridBpm[2] = {};
		double m_cacheElapsed[2] = { 0.0, 0.0 };
		std::shared_ptr<Waveform> m_cacheWave[2];
		double m_trueElapsed[2] = { 0.0, 0.0 };   // position reelle lue dans le moteur (par a-coups)
		                                          // (m_cacheElapsed = position affichee, lissee)
		ULONGLONG m_lastPaintTick = 0;            // heure du dernier dessin, pour lisser le defilement
		bool m_dragDirty=false;
        int m_idleTicks = 0;
		bool m_scrollActive[2][2] = {};
		ULONGLONG m_scrollStart[2][2] = {};
		int m_scrollWidth[2][2] = {};
		std::wstring m_scrollText[2][2];
		void DrawScrolling(CDCHandle dc, const CRect & area, const wchar_t * text, int deck, int line);

		CDjDropTarget * m_dropTarget = nullptr;   // cible de glisser-deposer enregistree sur la fenetre
		bool m_dropOk = false;                    // ce qui est glisse en ce moment est utilisable
		int m_dropHover = -1;                     // deck survole pendant un glisser (-1 = aucun)

		// --- Partage avec le thread du moteur : acces uniquement en tenant m_lock ---
		std::mutex m_lock;
		Deck m_deck[2];
		std::wstring m_error[2];   // message d'erreur affiche dans chaque deck (vide = pas d'erreur)
		unsigned m_outRate = 48000;   // frequence envoyee a la sortie audio

		// --- Valeurs atomiques lues par le moteur sans verrou ---
		std::atomic<unsigned> m_outputRatePreference{0};
        std::atomic<bool> m_limitEnabled{true};
        std::atomic<double> m_limitCeiling{-1},m_masterPeak{0},m_limitReduction{0};
        std::atomic<ULONGLONG> m_masterClipUntil{0},m_masterLimitUntil{0};
        std::atomic<float> m_crossfade{ 0.5f };   // position du crossfader : 0 = deck A seul, 1 = deck B seul
		std::atomic<bool> m_fixedPitch[2];
		std::atomic<double> m_speed[2];
        std::atomic<int> m_speedRange[2];
        std::atomic<int> m_transpose[2];
		std::atomic<float> m_volume[2];           // volume de chaque deck : 0 a 1
		std::atomic<int> m_stallMs{ 0 };          // depuis combien de ms la sortie audio n'est plus prete (0 = tout va bien)

		std::thread m_thread;
		std::atomic<bool> m_stop{ false };
		std::atomic<bool> m_engineFinished{ false };

	protected:
		// Doit rester "protected" pour que ui_element_impl_withpopup<> fonctionne.
		const ui_element_instance_callback_ptr m_callback;
	};

	void CDjPanel::notify(const GUID & p_what, t_size p_param1, const void * p_param2, t_size p_param2size) {
		if (p_what == ui_element_notify_colors_changed || p_what == ui_element_notify_font_changed) {
			// On utilise les couleurs et la police de foobar : on redessine quand elles changent.
			Invalidate();
		}
	}

	CDjPanel::CDjPanel(ui_element_config::ptr config, ui_element_instance_callback_ptr p_callback) : m_callback(p_callback), m_config(config) {
		g_english = cfg_language.get() != 0;
        dj_skin::flat=cfg_flat.get()!=0;
        g_replaygain=(cfg_replaygain.get()>=0&&cfg_replaygain.get()<=2)?cfg_replaygain.get():0;g_scrobble=cfg_scrobble.get()!=0;
        dj_skin::colorIndex[0]=(cfg_colorA.get()>=0&&cfg_colorA.get()<8)?cfg_colorA.get():0;
        dj_skin::colorIndex[1]=(cfg_colorB.get()>=0&&cfg_colorB.get()<8)?cfg_colorB.get():1;
		m_fixedPitch[0] = m_fixedPitch[1] = true;
		m_speed[0] = m_speed[1] = 1.0;
        m_speedRange[0] = m_speedRange[1] = 16;
        m_transpose[0] = m_transpose[1] = 0;
        m_cueVolume=std::clamp((int)cfg_cueVolume.get(),0,100)/100.;
		m_volume[0] = 1.0f;
		m_volume[1] = 1.0f;
	}

	void CDjPanel::OnDestroy() {
        m_midiInput.close();
        m_settings.close();m_settings.values={};m_settings.changed={};
        if(m_tooltip) { ::DestroyWindow(m_tooltip); m_tooltip=nullptr; }
		for(auto& window:m_queueWindow)if(window.m_hWnd)window.DestroyWindow();
		KillTimer(kTimerId);
        ClearPreparations(true);m_previewOverview.stop();m_previewPlayer.stop();m_previewActive=false;m_previewWindow.close();
        CancelStems();
		RevokeDrop();          // la fenetre n'accepte plus de depots
		StopEngine();          // arrete proprement le moteur avant que la fenetre disparaisse
		m_cueOutput.stop();
		SaveSession();
		StopWaveThread(0);
		StopWaveThread(1);
	}

	// Le fond est dessine dans OnPaint, avec le reste, en une seule fois : ici on ne fait rien,
	// pour eviter que l'ecran se vide un instant avant chaque redessin (c'est ce qui faisait clignoter).
	BOOL CDjPanel::OnEraseBkgnd(CDCHandle dc) {
		return TRUE;
	}

	// ---------------------------------------------------------------- curseurs, zoom, forme d'onde

	// Place le crossfader selon la position x de la souris (0 = tout a gauche = deck A, 1 = tout a droite = deck B).
	void CDjPanel::SetFaderFromX(int x, const CRect & client) {
        CancelAutomation();
		const int left = FaderLeft(client), right = FaderRight(client);
		double t = (right > left) ? (double)(x - left) / (double)(right - left) : 0.5;
		t = (std::min)(1.0, (std::max)(0.0, t));
		m_crossfade = (float)t;
		m_dragDirty=true;
	}

	// Place le volume d'un deck selon la position y de la souris (en bas = 0, en haut = 1).
	void CDjPanel::SetVolumeFromY(int deck, int y, const CRect & client) {
		const DeckLayout L = MakeLayout(deck, client);
		const int top = VolTrackTop(L), bottom = VolTrackBottom(L);
		double v = (bottom > top) ? (double)(bottom - y) / (double)(bottom - top) : 1.0;
		v = (std::min)(1.0, (std::max)(0.0, v));
		m_volume[deck] = (float)v;
		m_dragDirty=true;
	}

	// Fenetre visible de la forme d'onde : debut (t0) et duree (width), en secondes.
	// Quand on est zoome, la fenetre suit la tete de lecture (qui reste au centre, sauf aux extremites).

	void CDjPanel::EditGrid(int deck, int action, double step) {
		{
			std::lock_guard<std::mutex> lock(m_lock);
			const Deck & d = m_deck[deck];
			if (!d.loaded || !d.wave) return;
			const std::string key = GridKey(m_savedPath[deck], m_savedSubsong[deck]);
			if (action == 3) { m_manualGrid[deck] = false; m_manualPhase[deck] = 0; m_gridLibrary.erase(key); }
			else {
				const double position = d.framesPlayed / m_outRate;
				double anchor = position;
				if (action != 1) {
					if (m_manualGrid[deck]) anchor = m_manualPhase[deck];
					else { std::lock_guard<std::mutex> lockWave(d.wave->m); anchor = dj_beats::fit(d.wave->rms,d.bpmValue,position).offset; }
					anchor += action == 0 ? -step : step;
				}
				const auto grid = dj_beats::manual(d.bpmValue, anchor);
				if (!grid.valid) { m_error[deck] = L"Grille : tag BPM valide requis"; return; }
				m_manualGrid[deck] = true; m_manualPhase[deck] = grid.offset;
				m_gridLibrary[key] = picojson::value(grid.offset);
			}
			m_gridStamp[deck] = 0; m_error[deck].clear();
		}
		SaveSession();
	}

	void CDjPanel::SyncBeats(int deck) {
		{
			std::lock_guard<std::mutex> lock(m_lock);
			Deck & own = m_deck[deck]; const Deck & ref = m_deck[1-deck];
            if((own.loaded && !own.helper) || (ref.loaded && !ref.helper)) { m_error[deck]=g_english.load()?L"Finish analysis, then press Play":L"Terminez l'analyse puis cliquez Lecture"; return; }
			if (!own.loaded || !ref.loaded || !own.wave || !ref.wave) { m_error[deck] = L"Charge les deux platines"; return; }
			double speed = m_speed[deck].load();
			double factor=1; const auto match = dj_tempo::match(own.bpmValue, ref.bpmValue, m_speed[1-deck].load(), speed, m_speedRange[deck].load(), &factor);
			if (match != dj_tempo::Match::ok) { m_error[deck] = match == dj_tempo::Match::missing ? L"Deux tags BPM valides requis" : (g_english.load()?L"BPM outside selected range":L"BPM hors plage choisie"); return; }
			const double now = own.framesPlayed / m_outRate, refNow = ref.framesPlayed / m_outRate;
			dj_beats::Grid ownGrid, refGrid;
			{ std::lock_guard<std::mutex> lockWave(own.wave->m); ownGrid = dj_beats::fit(own.wave->rms, own.bpmValue, now); }
			{ std::lock_guard<std::mutex> lockWave(ref.wave->m); refGrid = dj_beats::fit(ref.wave->rms, ref.bpmValue, refNow); }
			if (m_manualGrid[deck]) ownGrid = dj_beats::manual(own.bpmValue,m_manualPhase[deck]);
			if (m_manualGrid[1-deck]) refGrid = dj_beats::manual(ref.bpmValue,m_manualPhase[1-deck]);
			if (!ownGrid.valid || !refGrid.valid) { m_error[deck] = L"SYNC : analyse en cours ou rythme incertain"; return; }
			// Align against the virtual reference grid at half/double BPM, preserving its anchor.
            refGrid.period/=factor;
            double target = now;
			if (!dj_beats::align(ownGrid, refGrid, now, refNow, own.length, target)) { m_error[deck] = L"SYNC : position incompatible"; return; }
			try {
				if (!own.stems && !own.helper->can_seek()) { m_error[deck] = L"SYNC : positionnement impossible"; return; }
				abort_callback_dummy abort; if(own.stems)own.stems->seek(target);else own.helper->seek(target, abort);
				own.src.clear(); own.srcPos = 0; own.pending.clear(); own.pendingPos = 0;
				own.eof = false; own.stretch.reset(); own.stretchFlushed = false;
				own.framesPlayed = target * m_outRate;
                if(own.loop.enabled&&!own.loop.contains(target))own.loop.enabled=false;
				m_speed[deck] = speed; m_error[deck].clear();
				m_gridStamp[0] = m_gridStamp[1] = 0;
			} catch (const std::exception & e) { pfc::stringcvt::string_wide_from_utf8 text(e.what()); m_error[deck] = text.get_ptr(); return; }
		}
		SaveSession();
	}

	void CDjPanel::MatchTempo(int deck) {
        if(m_pendingLoad[0]||m_pendingLoad[1])return;
		{
			std::lock_guard<std::mutex> lock(m_lock);
			if (!m_deck[0].loaded || !m_deck[1].loaded) { m_error[deck] = L"Charge les deux platines"; return; }
			double speed = m_speed[deck].load();
			const auto result = dj_tempo::match(m_deck[deck].bpmValue, m_deck[1-deck].bpmValue, m_speed[1-deck].load(), speed, m_speedRange[deck].load());
			if (result == dj_tempo::Match::missing) { m_error[deck] = L"Deux tags BPM valides requis"; return; }
			if (result == dj_tempo::Match::outside) { m_error[deck] = (g_english.load()?L"BPM outside selected range":L"BPM hors plage choisie"); return; }
			m_speed[deck] = speed; m_error[deck].clear();
		}
		SaveSession();
	}

	void CDjPanel::SetSpeedFromX(int deck, int x, const CRect & client) {
		const auto area = MakeLayout(deck, client).speed;
		const int left = area.left + 10, right = area.right - 10;
		if (right <= left) return;
		const double value = (std::max)(0.0, (std::min)(1.0, (double)(x-left)/(right-left)));
		const double r=dj_tempo::range(m_speedRange[deck].load());
        m_speed[deck] = 1-r + value * 2*r; Invalidate();
	}

	void CDjPanel::ViewWindow(int deck, double & t0, double & width) const {
		const double length = m_cacheLength[deck];
		width = (m_view[deck] > 0 && m_view[deck] < length) ? m_view[deck] : length;
		t0 = m_cacheElapsed[deck] - width / 2;
		t0 = (std::max)(0.0, (std::min)(t0, length - width));
	}

	void CDjPanel::ZoomDeck(int deck, bool zoomIn) {
		const double length = m_cacheLength[deck];
		if (length <= 0) return;
		double width = (m_view[deck] > 0 && m_view[deck] < length) ? m_view[deck] : length;
		if (zoomIn) width = (std::max)(3.0, width / 2);
		else width = width * 2;
		m_view[deck] = (width >= length) ? 0.0 : width;
		Invalidate();
	}

	// Clic sur la forme d'onde : on saute a l'instant correspondant.
	void CDjPanel::SeekFromWaveX(int deck, int x, const CRect & client) {
		if (!m_cacheLoaded[deck] || m_cacheLength[deck] <= 0) return;
		const DeckLayout L = MakeLayout(deck, client);
		const int pixels = L.wave.Width() - 2;
		if (pixels <= 0) return;
		double t0, width;
		ViewWindow(deck, t0, width);
		const double t = t0 + (double)(x - (L.wave.left + 1)) / (double)pixels * width;
		SeekDeck(deck, t);
	}


	void CDjPanel::DrawScrolling(CDCHandle dc, const CRect & area, const wchar_t * text, int deck, int line) {
		m_scrollActive[deck][line] = false;
		if (area.Width() <= 0 || area.Height() <= 0 || !text || !*text) return;
		SIZE extent = {}; GetTextExtentPoint32W(dc, text, (int)wcslen(text), &extent);
		const int overflow = extent.cx - area.Width();
		const ULONGLONG now = GetTickCount64();
		if (m_scrollText[deck][line] != text || m_scrollWidth[deck][line] != area.Width()) {
			m_scrollText[deck][line] = text; m_scrollWidth[deck][line] = area.Width(); m_scrollStart[deck][line] = now;
		}
		double offset = 0;
		if (overflow > 0) {
			m_scrollActive[deck][line] = true;
			const double pause = 1.5, travel = overflow / 28.0;
			const double phase = std::fmod((now - m_scrollStart[deck][line]) / 1000.0, 2 * (pause + travel));
			if (phase < pause) offset = 0;
			else if (phase < pause + travel) offset = (phase - pause) * 28.0;
			else if (phase < 2 * pause + travel) offset = overflow;
			else offset = overflow - (phase - 2 * pause - travel) * 28.0;
		}
		const int saved = dc.SaveDC();
		dc.IntersectClipRect(&area);
		CRect draw = area; draw.left -= (int)(offset + 0.5); draw.right = draw.left + (std::max)((int)extent.cx + 2, area.Width());
		dc.DrawText(text, -1, &draw, DT_NOPREFIX | DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		dc.RestoreDC(saved);
	}

	void CDjPanel::PaintWave(CDCHandle dc, const DeckLayout & L, int deck, COLORREF textColor, COLORREF bgColor) {
		if (!dc.RectVisible(&L.wave)) return;
		CBrush bright, dim, head, frame;
		bright.CreateSolidBrush(DeckAccent(deck));
		dim.CreateSolidBrush(Blend(DeckAccent(deck), bgColor, 0.65));
		head.CreateSolidBrush(kText);
		frame.CreateSolidBrush(Blend(textColor, bgColor, 0.35));

		CRect outer = L.wave;
		FillBox(dc, outer, RGB(12, 16, 22));
		dc.FrameRect(&outer, frame);
		for (int grid = 1; grid < 4; ++grid) {
			const int y = outer.top + outer.Height() * grid / 4;
			FillBox(dc, CRect(outer.left + 1, y, outer.right - 1, y + 1), RGB(30, 38, 49));
		}

		const double length = m_cacheLength[deck];
		std::shared_ptr<Waveform> wave = m_cacheWave[deck];
		if (!m_cacheLoaded[deck] || length <= 0 || !wave) return;

		const CRect inner(outer.left + 1, outer.top + 1, outer.right - 1, outer.bottom - 1);
		const int width = inner.Width();
		const int height = inner.Height();
		if (width <= 0 || height <= 2) return;

		const double elapsed = m_cacheElapsed[deck];
		double t0, span;
		ViewWindow(deck, t0, span);

		// Pour chaque colonne de pixels : le pic le plus haut sur la tranche de temps correspondante.
		std::vector<float> columns((size_t)width, 0.0f), body((size_t)width, 0.0f);
		{
			std::lock_guard<std::mutex> lock(wave->m);
			const size_t n = wave->peaks.size();
			const ULONGLONG now = GetTickCount64();
			if (m_gridSource[deck] != wave || m_gridBpm[deck] != m_cacheBpm[deck] || !m_gridStamp[deck] || now - m_gridStamp[deck] >= 1000 || std::abs(elapsed-m_gridPosition[deck]) > 0.75) {
				m_grid[deck] = dj_beats::fit(wave->rms, m_cacheBpm[deck], elapsed);
				m_gridSource[deck] = wave; m_gridStamp[deck] = now;
				m_gridPosition[deck] = elapsed; m_gridBpm[deck] = m_cacheBpm[deck];
			}
			for (int x = 0; x < width; ++x) {
				const double ta = t0 + span * (double)x / (double)width;
				const double tb = t0 + span * (double)(x + 1) / (double)width;
				size_t ia = (size_t)(ta * kPeaksPerSecond);
				size_t ib = (size_t)(tb * kPeaksPerSecond);
				if (ib <= ia) ib = ia + 1;
				if (ib > n) ib = n;
				float m = 0.0f;
				for (size_t k = ia; k < ib; ++k) m = (std::max)(m, wave->peaks[k]);
				columns[(size_t)x] = m;
				double energy = 0;
				for (size_t k = ia; k < ib && k < wave->rms.size(); ++k) energy += (double)wave->rms[k] * wave->rms[k];
				if (ib > ia) body[(size_t)x] = (float)std::sqrt(energy / (ib - ia));
			}
		}

		const int cy = (inner.top + inner.bottom) / 2;
		const int halfH = height / 2;
		CBrush outlinePast, outlineFuture;
		outlinePast.CreateSolidBrush(Blend(DeckAccent(deck), bgColor, 0.86));
		outlineFuture.CreateSolidBrush(Blend(DeckAccent(deck), bgColor, 0.64));
		for (int x = 0; x < width; ++x) {
			const float peak = columns[(size_t)x];
			if (peak <= 0.0f) continue;
			int h = (int)((std::min)(1.0f, peak) * (float)halfH * 0.88f); // linear peak outline; keep vertical headroom
			if (h < 1) h = 1;
			const double tx = t0 + span * ((double)x + 0.5) / (double)width;
			CRect bar(inner.left + x, cy - h, inner.left + x + 1, cy + h + 1);
			dc.FillRect(&bar, tx < elapsed ? outlinePast : outlineFuture);
			const int coreH = (std::max)(1, (int)((std::min)(1.0f, body[(size_t)x]) * halfH * 0.88f));
			CRect core(inner.left + x, cy - coreH, inner.left + x + 1, cy + coreH + 1);
			dc.FillRect(&core, (tx < elapsed) ? dim : bright);
		}

		// Solid: audio phase accepted by SYNC. Dotted: tagged spacing only, never a SYNC guarantee.
		bool indicative=false;
        const bool manual=m_manualGrid[deck];
        const auto displayGrid=dj_beats::displayed(m_grid[deck],m_cacheBpm[deck],manual,m_manualPhase[deck],indicative);
        const bool detected=!indicative;
		if (displayGrid.valid) {
			const auto ticks = dj_beats::ticks(displayGrid, t0, span, width);
			const COLORREF markerColor = detected ? Blend(DeckAccent(deck), kText, 0.45) : RGB(130,148,169);
			CBrush line, cap; line.CreateSolidBrush(Blend(markerColor, kCanvas, 0.28)); cap.CreateSolidBrush(markerColor);
			CPen dots; dots.CreatePen(PS_DOT,1,markerColor);
			const HPEN oldPen = dc.SelectPen(dots);
			for (const int x : ticks.pixels) {
				const int px = inner.left+x;
				if (detected) { CRect marker(px,inner.top+18,px+1,inner.bottom); dc.FillRect(&marker,line); }
				else { dc.MoveTo(px,inner.top+18); dc.LineTo(px,inner.bottom); }
				CRect top(px,inner.top+18,px+2,inner.top+25), bottom(px,inner.bottom-7,px+2,inner.bottom);
				dc.FillRect(&top,cap); dc.FillRect(&bottom,cap);
			}
			dc.SelectPen(oldPen);
			wchar_t text[128];
			if (ticks.stride > 1) swprintf_s(text,detected ? (manual ? Tr(L"Grille manuelle | 1 trait / %u temps") : Tr(L"Battements estim\u00e9s | 1 trait / %u temps")) : Tr(L"Grille BPM indicative | 1 trait / %u temps"),ticks.stride);
			else swprintf_s(text,detected ? (manual ? Tr(L"Grille manuelle") : Tr(L"Battements estim\u00e9s")) : Tr(L"Grille BPM indicative (phase incertaine)"));
			CFont markerFont; markerFont.CreatePointFont(70,L"Segoe UI"); SelectObjectScope markerScope(dc,(HGDIOBJ)(HFONT)markerFont);
			CRect label(inner.left+3,inner.top+3,inner.right-3,inner.top+18);
			dc.SetBkMode(OPAQUE); dc.SetBkColor(RGB(12,16,22)); dc.SetTextColor(markerColor);
			dc.DrawText(text,-1,&label,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS); dc.SetBkMode(TRANSPARENT);
		} else {
			CRect label(inner.left+3,inner.top+3,inner.right-3,inner.top+18);
			dc.SetTextColor(kMuted); dc.DrawText(Tr(L"Rep\u00e8res : tag BPM valide requis"),-1,&label,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
		}

		// Tete de lecture
		const int px = inner.left + (int)((elapsed - t0) / span * (double)width);
		if (px >= inner.left && px < inner.right) {
			CRect playhead(px - 1, inner.top, px + 1, inner.bottom);
			dc.FillRect(&playhead, head);
		}
        const auto& region=m_cacheLoop[deck];
        for(int k=0;k<2;++k){double t=k?region.b:region.a;if(t<0||t<t0||t>t0+span)continue;
            int x=inner.left+(int)((t-t0)*width/span);x=(std::min)((int)inner.right-2,(std::max)((int)inner.left,x));
            FillBox(dc,CRect(x,inner.top,x+2,inner.bottom),region.enabled?RGB(120,255,160):kText);
            CRect label(x+3,inner.top+18,(std::min)((int)inner.right,x+22),inner.top+36);dc.SetTextColor(kText);dc.DrawText(k?L"B":L"A",-1,&label,DT_SINGLELINE);
        }

	}

	// ---------------------------------------------------------------- dessin

	// Dessin "hors ecran" : tout est dessine dans une image en memoire, puis copie d'un coup a l'ecran.
	// L'ecran n'affiche donc jamais un dessin a moitie fait.
	void CDjPanel::OnPaint(CDCHandle) {
		CPaintDC dc(*this);
		CRect rc;
		GetClientRect(&rc);
		if (rc.Width() <= 0 || rc.Height() <= 0) return;

		CDC mem;
		mem.CreateCompatibleDC(dc);
		if(!m_paintBuffer.m_hBitmap||m_paintWidth!=rc.Width()||m_paintHeight!=rc.Height()){m_paintBuffer.DeleteObject();m_paintBuffer.CreateCompatibleBitmap(dc,rc.Width(),rc.Height());m_paintWidth=rc.Width();m_paintHeight=rc.Height();}
        HBITMAP oldBitmap = mem.SelectBitmap(m_paintBuffer);
		CRect dirty; dc.GetClipBox(&dirty); mem.IntersectClipRect(&dirty);

		CBrush background;
		background.CreateSolidBrush(kCanvas);
		mem.FillRect(&rc, background);

		PaintAll(CDCHandle(mem), rc);

		dc.BitBlt(0, 0, rc.Width(), rc.Height(), mem, 0, 0, SRCCOPY);
		mem.SelectBitmap(oldBitmap);
	}

    void CDjPanel::UpdateTooltips(const CRect& client) {
        const bool en=g_english.load();
        if(m_tooltip && client==m_tipClient && en==m_tipEnglish) return;
        if(!m_tooltip) {
            m_tooltip=CreateWindowExW(WS_EX_TOPMOST,TOOLTIPS_CLASSW,nullptr,dj_tooltip::style,
                CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,m_hWnd,nullptr,core_api::get_my_instance(),nullptr);
            if(!m_tooltip) return;
            SendMessageW(m_tooltip,TTM_SETMAXTIPWIDTH,0,420);
            SendMessageW(m_tooltip,TTM_SETDELAYTIME,TTDT_INITIAL,dj_tooltip::initial);
            SendMessageW(m_tooltip,TTM_SETDELAYTIME,TTDT_AUTOPOP,dj_tooltip::autopop);
            SendMessageW(m_tooltip,TTM_SETDELAYTIME,TTDT_RESHOW,dj_tooltip::reshow);
        }
        for(size_t n=0;n<m_tips.size();++n) { TOOLINFOW ti={sizeof(ti)}; ti.hwnd=m_hWnd;ti.uId=n+1;SendMessageW(m_tooltip,TTM_DELTOOLW,0,(LPARAM)&ti); }
        m_tips.clear(); m_tipClient=client;m_tipEnglish=en;
        auto add=[&](CRect r,const wchar_t* fr,const wchar_t* english){if(r.Width()>0 && r.Height()>0)m_tips.push_back({r,en?english:fr});};
        for(int i=0;i<2;++i) {
            const auto L=MakeLayout(i,client);
            add(L.stemPrepare,L"Stems expérimentaux : préparer en local (modèle Demucs), progression/cache ; cliquer pour annuler. Une fois prêts : menu pour activer les quatre ou revenir au son original.",L"Experimental stems: prepare locally (Demucs model), progress/cache; click to cancel. When ready: menu to enable all four or return to original audio.");
            const wchar_t* stemFr[]={L"Batterie/percussions",L"Basse",L"Autres instruments",L"Voix"};
            const wchar_t* stemEn[]={L"Drums/percussion",L"Bass",L"Other instruments",L"Vocals"};
            for(int k=0;k<4;++k){
                add(L.stem[k],stemFr[k],stemEn[k]);
                add(L.stemSolo[k],L"Solo : écouter uniquement ce stem. Plusieurs solos peuvent être combinés. Désactiver tous les solos restaure les coupures précédentes.",L"Solo: hear only this stem. Multiple solos may be combined. Clearing all solos restores previous mutes.");
                add(L.stemVolume[k],L"Volume du stem : glisser, ou molette (5 %, Maj : 1 %). Clic droit : 100 %. Le volume ne réactive pas un stem coupé.",L"Stem volume: drag, or wheel (5%, Shift: 1%). Right-click: 100%. Volume does not unmute a stem.");
            }
            add(L.headphones,L"Préécoute casque avant volume/crossfader. Le deck doit jouer ; couper sa contribution au mix avec le crossfader si nécessaire. Sortie et volume dans Réglages → Préécoute casque.",L"Headphone cue before deck volume/crossfader. The deck must play; use the crossfader to keep it out of the master if needed. Output and volume in Settings → Headphone cue.");
            add(L.automation,L"Lecture automatique : enchaînement de cette liste, alternance A/B, durée et courbe du fondu.",L"Automatic playback: this playlist, alternating A/B, fade duration and curve.");
            add(L.queueOpen,L"Ouvrir la liste de cette platine : glisser des morceaux, reordonner, supprimer. Double-clic pour charger.",L"Open this deck's playlist: drop tracks, reorder, remove. Double-click to load.");
            add(L.release,L"D\u00e9charger : arr\u00eater et fermer le fichier. Position conserv\u00e9e ; Lecture reprend au m\u00eame endroit. Lib\u00e8re aussi l'autre platine si elle utilise ce fichier.",L"Unload: stop and close the file. Retains position; Play resumes there. Also releases the other deck if it uses this file.");
            add(L.analyse[0],L"Calculer le BPM avec l’analyseur choisi dans Réglages → Audio (Auto : Smart Tempo puis foo_bpm). La platine est mise en pause et le fichier lib\u00e9r\u00e9 pour enregistrer les tags. Reprendre apr\u00e8s validation.",L"Calculate BPM using Settings → Audio analyser (Auto: Smart Tempo, then foo_bpm). Pauses and releases the file for tag writing. Resume after saving the result.");
            add(L.analyse[1],L"Calculer la tonalit\u00e9 avec foo_musical_key. Reprendre la lecture apr\u00e8s la fin de l'analyse et l'enregistrement des tags.",L"Calculate key using foo_musical_key. Resume after analysis and tag writing have finished.");
            add(L.analyse[2],L"Actualiser les tags : relire le BPM et la tonalit\u00e9 et afficher les valeurs. Ne lance pas d'analyse audio.",L"Refresh tags: read BPM and key and show their values. Does not analyse audio.");
            add(L.loopSnap,L"Q : aligner A/B sur le repère affiché le plus proche (selon le zoom). Grille manuelle ou estimée ; pointillés : phase indicative. Activer Q recale aussi les points existants et les sauvegarde.",L"Q: snap A/B to the nearest displayed line (zoom-dependent). Manual/estimated grid; dotted lines have indicative phase. Enabling Q also snaps and saves existing points.");
            add(L.loop[0],L"Poser le début A à la position actuelle. Boucle minimale : 100 ms.",L"Set loop start A at the current position. Minimum loop: 100 ms.");
            add(L.loop[1],L"Poser la fin B à la position actuelle, après A. Activer ensuite la boucle.",L"Set loop end B at the current position, after A. Then enable the loop.");
            add(L.loop[2],L"Activer/désactiver la boucle A/B. AUTO attend pendant la boucle. Un clic hors de la boucle la désactive.",L"Enable/disable the A/B loop. AUTO waits while looping. Seeking outside disables it.");
            add(L.loop[3],L"Effacer les points A/B de cette platine.",L"Clear this deck’s A/B points.");
            add(L.gridControl[0],L"D\u00e9caler la grille de -10 ms. Maj : -1 ms.",L"Shift beat grid -10 ms. Shift: -1 ms.");
            add(L.gridControl[1],L"Temps ici : ancrer un battement sur la position actuelle.",L"Beat here: anchor the grid at the current position.");
            add(L.gridControl[2],L"D\u00e9caler la grille de +10 ms. Maj : +1 ms.",L"Shift beat grid +10 ms. Shift: +1 ms.");
            add(L.gridControl[3],L"Grille automatique : supprimer le r\u00e9glage manuel du morceau.",L"Automatic grid: remove this track's manual override.");
            add(L.pitch[0],L"Baisser la tonalit\u00e9 d'un demi-ton, sans changer le tempo.",L"Transpose down one semitone without changing tempo.");
            add(L.pitch[1],L"Transposition actuelle. Cliquer pour revenir \u00e0 z\u00e9ro.",L"Current transposition. Click to reset to zero.");
            add(L.pitch[2],L"Monter la tonalit\u00e9 d'un demi-ton, sans changer le tempo.",L"Transpose up one semitone without changing tempo.");
            add(L.matchKey,i==0?L"Caler la tonalit\u00e9 sur B sans changer le tempo. M\u00eame tonalit\u00e9, ou relative compatible si les modes diff\u00e8rent.":L"Caler la tonalit\u00e9 sur A sans changer le tempo. M\u00eame tonalit\u00e9, ou relative compatible si les modes diff\u00e8rent.",i==0?L"Match key to B without changing tempo. Same key, or relative compatible key for opposite modes.":L"Match key to A without changing tempo. Same key, or relative compatible key for opposite modes.");
            add(L.key,L"Tonalit\u00e9 fixe / mode vinyle. Cadenas ferm\u00e9 : changer le tempo en conservant la hauteur.",L"Key lock / vinyl mode. Closed lock: tempo changes preserve pitch.");
            add(L.beat,i==0?L"SYNC vers B : caler BPM et phase des battements. Calage ponctuel exp\u00e9rimental.":L"SYNC vers A : caler BPM et phase des battements. Calage ponctuel exp\u00e9rimental.",i==0?L"SYNC to B: match BPM and beat phase. Experimental one-shot alignment.":L"SYNC to A: match BPM and beat phase. Experimental one-shot alignment.");
            add(L.sync,i==0?L"Caler le BPM sur B : exact, moitié ou double, avec le plus petit changement de vitesse dans la plage choisie.":L"Caler le BPM sur A : exact, moitié ou double, avec le plus petit changement de vitesse dans la plage choisie.",i==0?L"Match B: exact, half or double BPM, choosing the smallest speed change within range.":L"Match A: exact, half or double BPM, choosing the smallest speed change within range.");
            add(L.range,L"Choisir la plage de vitesse : +/-8 %, +/-16 % ou +/-50 %.",L"Choose speed range: +/-8%, +/-16% or +/-50%.");
            CRect slider=L.speed;slider.top+=34;
            add(slider,L"Vitesse : glisser ou molette. Maj : r\u00e9glage fin. Clic droit : vitesse normale.",L"Speed: drag or wheel. Shift: fine adjustment. Right-click: normal speed.");
            add(L.zoomOut,L"R\u00e9duire le zoom de la forme d'onde.",L"Zoom waveform out.");
            add(L.zoomIn,L"Agrandir la forme d'onde.",L"Zoom waveform in.");
            add(L.vol,L"Volume de cette platine.",L"Volume for this deck.");
            add(L.btn[0],L"Lecture / pause. Apr\u00e8s une analyse, rouvrir le fichier \u00e0 la position conserv\u00e9e.",L"Play / pause. After analysis, reopen the file at the retained position.");
            add(L.btn[1],L"Retour au d\u00e9but du morceau.",L"Return to the beginning of the track.");
        }
        add(PlayBothRect(client),L"Inverser les deux \u00e9tats : lecture ensemble, pause ensemble ou \u00e9change de la platine en lecture.",L"Toggle both states: play together, pause together or swap the playing deck.");
        CRect fader=FaderStrip(client);fader.left=FaderLeft(client)-10;fader.right=FaderRight(client)+10;
        add(fader,L"Crossfader : doser les platines A et B.",L"Crossfader: blend decks A and B.");
        add(MasterMeterRect(client),L"Mix principal : vert = niveau normal ; orange = réduction du limiteur ; rouge = dépassement de 0 dBFS sans limiteur ou signal invalide. Plafond et activation dans Réglages → Audio. Limiteur de crêtes échantillonnées, pas true peak. Préécoute casque indépendante.",L"Master mix: green = normal; amber = limiter gain reduction; red = over 0 dBFS with limiter bypassed or invalid signal. Enable/ceiling in Settings → Audio. Sample-peak limiter, not true peak. Headphone cue is independent.");
        add(LanguageRect(client,false),L"Réglages : fenêtre flottante à onglets, apparence, audio, casque, playlists, lecture et stems. DJ Decks " FOO_DJ_VERSION_W,L"Settings: floating tabs for appearance, audio, headphones, playlists, playback and stems. DJ Decks " FOO_DJ_VERSION_W);
        add(LanguageRect(client,true),L"Interface et infobulles en anglais.",L"English interface and tooltips.");
        for(size_t n=0;n<m_tips.size();++n) {
            TOOLINFOW ti={sizeof(ti)};ti.uFlags=TTF_SUBCLASS;ti.hwnd=m_hWnd;ti.uId=n+1;ti.rect=m_tips[n].rect;ti.lpszText=LPSTR_TEXTCALLBACKW;
            SendMessageW(m_tooltip,TTM_ADDTOOLW,0,(LPARAM)&ti);
        }
    }

	void CDjPanel::PaintAll(CDCHandle dc, const CRect & rc) {
        UpdateTooltips(rc);
		const COLORREF textColor = kText;
		const COLORREF bgColor = kCanvas;
		dc.SetTextColor(textColor);
		dc.SetBkMode(TRANSPARENT);
		SelectObjectScope fontScope(dc, (HGDIOBJ)m_callback->query_font_ex(ui_font_default));

		CBrush frame;
		frame.CreateSolidBrush(RGB(55, 66, 82));

		static const wchar_t * titles[2] = { L"DECK A", L"DECK B" };

		// Temps ecoule depuis le dernier dessin : sert a faire avancer la tete de lecture en douceur.
		const ULONGLONG nowTick = GetTickCount64();
		double dt = m_lastPaintTick ? (double)(nowTick - m_lastPaintTick) / 1000.0 : 0.0;
		if (dt > 0.25) dt = 0.25;
		m_lastPaintTick = nowTick;

		for (int i = 0; i < 2; ++i) {
			// On recopie ce qu'il faut afficher pendant que le moteur ne touche pas aux decks.
			// Si le moteur est occupe, on garde l'affichage precedent plutot que de faire attendre l'interface.
			{
				std::unique_lock<std::mutex> lock(m_lock, std::try_to_lock);
				if (lock.owns_lock()) {
					const Deck & dk = m_deck[i];
                    for (int k=0;k<2;++k) m_cacheKey[k] = m_deck[k].loaded ? m_deck[k].musicalKey : L"";
					m_cachePlaying[i] = dk.playing;
					m_cacheLoaded[i] = dk.loaded;
                    m_cacheReleased[i] = dk.loaded && !dk.helper;
					m_cacheLength[i] = dk.length;
					m_cacheBpm[i] = dk.bpmValue;
					m_trueElapsed[i] = (double)dk.framesPlayed / (double)(m_outRate?m_outRate:48000);
					m_cacheWave[i] = dk.wave; m_cacheLoop[i]=dk.loop;

					std::wstring text;
					if (!m_error[i].empty()) {
						text = dj_language::message(m_error[i],g_english.load());
					} else if (!m_loopNotice[i].empty()) {
                        text=m_loopNotice[i];
                    } else if (dk.loaded) {
						wchar_t extra[96];
						if (dk.rate != m_outRate) {
							swprintf_s(extra, L"  -  %u Hz  ->  %u Hz", dk.rate, m_outRate);
						} else {
							swprintf_s(extra, L"  -  %u Hz", dk.rate);
						}
						text = FormatTime(m_trueElapsed[i]) + L" / " + FormatTime(dk.length);
						if (!dk.bpm.empty()) {
							if (dk.bpmValue > 0) { wchar_t tempo[64]; swprintf_s(tempo, L"  |  %.1f BPM", dk.bpmValue * m_speed[i].load()); text += tempo; }
							else text += L"  |  " + dk.bpm + L" BPM";
						}
						// Musical key is drawn separately as a colored compatibility badge.
						text += extra;
					}
					m_cacheInfo[i] = text;
				}
			}
			// Position affichee : le moteur ne la met a jour que par a-coups (un bloc de son a la fois).
			// En lecture, on la fait avancer au rythme de l'horloge, et on la rapproche doucement de la
			// vraie position ; en cas de gros ecart (saut dans le morceau, nouveau fichier), on recale d'un coup.
			if (m_cachePlaying[i]) {
				double shown = m_cacheElapsed[i] + dt * m_speed[i].load();
				const double err = m_trueElapsed[i] - shown;
				if (m_cacheLoop[i].enabled || std::fabs(err) > 0.3) shown = m_trueElapsed[i];
				else shown += err * 0.15;
				m_cacheElapsed[i] = shown;
			} else {
				m_cacheElapsed[i] = m_trueElapsed[i];
			}

			std::wstring info = m_cacheInfo[i];
			const bool playing = m_cachePlaying[i];
			const int stall = m_stallMs;
			if (playing && stall > 1500) {
				wchar_t t[64];
				swprintf_s(t, Tr(L"Sortie audio bloqu\u00e9e depuis %d s"), stall / 1000);
				info = t;
			}

			DeckLayout L = MakeLayout(i, rc);
			const COLORREF accent = DeckAccent(i);
            dj_skin::surface(dc,L.deck,accent,true);
            CRect display(L.deck.left+8,L.deck.top+7,L.deck.right-8,L.deck.top+117);
            dj_skin::surface(dc,display,RGB(50,60,68));
            DrawControl(dc,L.vol,RGB(8,10,14),RGB(65,72,82));
			if (m_dropHover == i) {      // un morceau est glisse au-dessus de ce deck : cadre rouge epais
				CBrush hot;
				hot.CreateSolidBrush(accent);
				CRect inner1 = L.deck, inner2 = L.deck;
				inner1.DeflateRect(1, 1);
				inner2.DeflateRect(2, 2);
				dc.FrameRect(&inner1, hot);
				dc.FrameRect(&inner2, hot);
			}

			// Cover stays with the loaded track, including during pause.
			DrawControl(dc, L.cover, kCanvas, Blend(accent, kSurface, 0.6));
			std::shared_ptr<CImage> cover;
			if (m_cacheWave[i]) { std::lock_guard<std::mutex> lock(m_cacheWave[i]->m); cover = m_cacheWave[i]->cover; }
			if (cover) cover->Draw(dc, L.cover.left + 1, L.cover.top + 1, L.cover.Width() - 2, L.cover.Height() - 2);
			else { CRect empty = L.cover; dc.SetTextColor(kMuted); dc.DrawText(L"\u266b", -1, &empty, DT_CENTER | DT_VCENTER | DT_SINGLELINE); }

			// Titre, nom du fichier, temps
			{
				CFont titleFont; titleFont.CreatePointFont(110, L"Segoe UI Semibold");
				SelectObjectScope titleScope(dc, (HGDIOBJ)(HFONT)titleFont);
				dc.SetTextColor(accent);
				const wchar_t* state=playing?Tr(L"LECTURE"):(m_cacheReleased[i]?(g_english.load()?L"FREE":L"LIBRE"):(m_cacheLoaded[i]?Tr(L"PR\u00caT"):Tr(L"VIDE")));
                SIZE extent={};dc.GetTextExtent(state,(int)wcslen(state),&extent);
                CRect status=HeaderStatusRect(L,(int)extent.cx);
                CRect heading=L.title;heading.right=status.left-8;
                dc.DrawText(titles[i],-1,&heading,DT_NOPREFIX|DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
                dc.DrawText(state,-1,&status,DT_NOPREFIX|DT_RIGHT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
			}
			dc.SetTextColor(textColor);
			const wchar_t * label = m_name[i].empty() ? Tr(L"Glisse un morceau ici") : m_name[i].c_str();
			{
				CFont songFont; songFont.CreatePointFont(95, L"Segoe UI Semibold");
				SelectObjectScope songScope(dc, (HGDIOBJ)(HFONT)songFont);
				DrawScrolling(dc, L.name, label, i, 0);
			}
			dc.SetTextColor(kMuted);
			DrawScrolling(dc, L.artist, m_artist[i].c_str(), i, 1);
			dc.SetTextColor(kMuted);
            CRect infoRect=L.info;
            const bool stable=(m_fixedPitch[0].load() || std::abs(m_speed[0].load()-1.0)<0.000001)
                && (m_fixedPitch[1].load() || std::abs(m_speed[1].load()-1.0)<0.000001);
            const auto harmony=dj_key::compatibility(dj_key::transpose(m_cacheKey[0],m_transpose[0].load()),dj_key::transpose(m_cacheKey[1],m_transpose[1].load()),stable);
            std::wstring keyText=dj_key::label(dj_key::transpose(m_cacheKey[i],m_transpose[i].load()));
            if (keyText.empty()) keyText=L"?";
            keyText += harmony==dj_key::Compatibility::usual ? L"  OK" : harmony==dj_key::Compatibility::other ? L"  ~" : L"  ?";
            SIZE keySize={}; dc.GetTextExtent(keyText.c_str(),(int)keyText.size(),&keySize);
            CRect keyRect=L.info;
            const int keyWidth=(std::min)((int)keySize.cx+14,(std::max)(50,L.info.Width()/2));
            keyRect.left=keyRect.right-keyWidth;
            infoRect.right=keyRect.left-6;
            const COLORREF harmonyColor=harmony==dj_key::Compatibility::usual ? RGB(95,220,145)
                : harmony==dj_key::Compatibility::other ? RGB(255,180,75) : kMuted;
            DrawControl(dc,keyRect,RGB(19,24,32),harmonyColor);
            dc.SetTextColor(harmonyColor);
            dc.DrawText(keyText.c_str(),-1,&keyRect,DT_NOPREFIX|DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            dc.SetTextColor(kMuted);
            if (!info.empty()) dc.DrawText(info.c_str(),-1,&infoRect,DT_NOPREFIX|DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

			DrawControl(dc,L.loopSnap,RGB(8,10,14),m_loopSnap[i]?accent:kMuted);dc.SetTextColor(m_loopSnap[i]?accent:kMuted);dc.DrawText(L"Q",-1,&L.loopSnap,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            // Independent loop controls, with millisecond positions in the two point buttons.
            for(int k=0;k<4;++k){
                const auto& loop=m_cacheLoop[i];bool on=k==0?loop.a>=0:k==1?loop.b>=0:k==2?loop.enabled:false;
                DrawControl(dc,L.loop[k],RGB(8,10,14),on?accent:kMuted);dc.SetTextColor(on?accent:kText);
                std::wstring label=k==0?L"A":k==1?L"B":k==2?(loop.enabled?L"A-B ON":L"A-B OFF"):L"×";
                if(k<2){double t=k==0?loop.a:loop.b;if(t>=0){wchar_t v[40];swprintf_s(v,L" %d:%05.2f",(int)(t/60),std::fmod(t,60.));label+=v;}}
                dc.DrawText(label.c_str(),-1,&L.loop[k],DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            }
            // Semitone transposition is independent of tempo; centre resets to zero.
            for(int k=0;k<3;++k) DrawControl(dc,L.pitch[k],RGB(37,45,58),accent);
            dc.SetTextColor(kText);
            dc.DrawText(L"-",-1,&L.pitch[0],DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            dc.DrawText(L"+",-1,&L.pitch[2],DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            wchar_t pitchLabel[80];
            swprintf_s(pitchLabel,g_english.load()?L"%+d st":L"%+d demi-tons",m_transpose[i].load());
            dc.DrawText(pitchLabel,-1,&L.pitch[1],DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            DrawControl(dc,L.matchKey,RGB(37,45,58),accent);
            dj_skin::icon(dc,L.matchKey,dj_skin::Icon::keyMatch);
            dc.SetTextColor(kMuted);
            for(int k=0;k<3;++k) {
                DrawControl(dc,L.analyse[k],RGB(37,45,58),accent);
                dc.SetTextColor(accent);
                dj_skin::icon(dc,L.analyse[k],k==0?dj_skin::Icon::pulse:k==1?dj_skin::Icon::keySearch:dj_skin::Icon::refresh);
            }
            for(int line=0;line<2;++line) {
                int index=line==0?m_queueCurrent[i]:(m_queueCurrent[i]+1);
                std::wstring text=line==0?(g_english.load()?L"Current: ":L"Actuel : "):(g_english.load()?L"Next: ":L"Suivant : ");
                if(index>=0 && index<(int)m_queueLabels[i].size())text+=m_queueLabels[i][index];
                else if(line==0 && !m_savedPath[i].empty())text+=m_name[i];
                else text+=L"—";
                FillBox(dc,L.queue[line],RGB(10,15,20));dc.SetTextColor(line? kText:accent);
                dc.DrawText(text.c_str(),-1,&L.queue[line],DT_NOPREFIX|DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
            }
            DrawControl(dc,L.queueOpen,RGB(8,10,14),accent);dc.SetTextColor(kText);
            dc.DrawText(L"≡",-1,&L.queueOpen,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            DrawControl(dc,L.stemPrepare,RGB(8,10,14),accent);dc.SetTextColor(m_stemReady[i]?accent:kMuted);
            std::wstring preparation=m_stemReady[i]?L"STEMS ✓":L"STEMS";
            if(m_stemActive[i]){unsigned count=0;for(int k=0;k<4;++k)if(m_stemMask[i]&(1<<k))++count;preparation=L"STEMS "+std::to_wstring(count)+L"/4";}
            if(m_stemJob && m_stemOwner==i)preparation=std::to_wstring(m_stemJob->progress.load())+L" % ×";
            dc.DrawText(preparation.c_str(),-1,&L.stemPrepare,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            const wchar_t* stemNamesFr[]={L"BAT",L"BASSE",L"INST",L"VOIX"};
            const wchar_t* stemNamesEn[]={L"DRUM",L"BASS",L"INST",L"VOC"};
            for(int k=0;k<4;++k){
                bool on=m_stemActive[i]&&(m_stemSolo[i]?(m_stemSolo[i]&(1<<k)):(m_stemMask[i]&(1<<k)));
                DrawControl(dc,L.stem[k],RGB(8,10,14),on?accent:RGB(65,72,80));dc.SetTextColor(on?accent:kMuted);
                dc.DrawText(g_english.load()?stemNamesEn[k]:stemNamesFr[k],-1,&L.stem[k],DT_CENTER|DT_VCENTER|DT_SINGLELINE);
                DrawControl(dc,L.stemSolo[k],RGB(8,10,14),(m_stemSolo[i]&(1<<k))?accent:kMuted);
                dc.SetTextColor((m_stemSolo[i]&(1<<k))?accent:kMuted);dc.DrawText(L"S",-1,&L.stemSolo[k],DT_CENTER|DT_VCENTER|DT_SINGLELINE);
                auto r=L.stemVolume[k];FillBox(dc,r,RGB(10,15,20));
                int y=r.bottom-3,x=r.left+(int)(m_stemVolume[i][k]*(r.Width()-1));
                FillBox(dc,CRect(r.left,y,r.right,y+2),RGB(60,70,80));FillBox(dc,CRect(r.left,y,x+1,y+2),accent);
                wchar_t level[16];swprintf_s(level,L"%.0f%%",m_stemVolume[i][k]*100);dc.SetTextColor(kText);dc.DrawText(level,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            }
            DrawControl(dc,L.headphones,RGB(8,10,14),m_cue[i].load()?accent:kMuted);
            dj_skin::icon(dc,L.headphones,dj_skin::Icon::headphones,m_cue[i].load()?accent:kMuted);
            DrawControl(dc,L.automation,RGB(8,10,14),accent);dc.SetTextColor(m_alternate||m_chain[i]?accent:kText);
            dc.DrawText(m_alternate?L"A ↔ B":m_chain[i]?L"AUTO ▶":L"AUTO",-1,&L.automation,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            DrawControl(dc,L.release,RGB(8,10,14),accent);dj_skin::icon(dc,L.release,dj_skin::Icon::release);
            dc.SetTextColor(kText);
            // Boutons de zoom
			DrawControl(dc, L.zoomOut, RGB(37, 45, 58), RGB(55, 66, 82));
			dc.DrawText(L"-", -1, &L.zoomOut, DT_NOPREFIX | DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			DrawControl(dc, L.zoomIn, RGB(37, 45, 58), RGB(55, 66, 82));
			dc.DrawText(L"+", -1, &L.zoomIn, DT_NOPREFIX | DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			// Forme d'onde
			PaintWave(dc, L, i, accent, bgColor);

			// Curseur de volume : un mot "VOL", une glissiere verticale et un curseur
			{
				CRect volLabel(L.vol.left, L.vol.top, L.vol.right, L.vol.top + 18);
				dj_skin::icon(dc,volLabel,dj_skin::Icon::speaker);
				const int top = VolTrackTop(L), bottom = VolTrackBottom(L);
				const int cx = (L.vol.left + L.vol.right) / 2;
				if (bottom > top) {
					CRect track(cx - 1, top, cx + 1, bottom);
					FillBox(dc, CRect(cx - 3, top, cx + 3, bottom), RGB(9, 13, 19));
					const int hy = bottom - (int)((bottom - top) * m_volume[i].load() + 0.5f);
					CRect handle(cx - 12, hy - 5, cx + 12, hy + 5);
					FillBox(dc, CRect(cx - 2, hy, cx + 2, bottom), accent);
					DrawControl(dc, handle, RGB(203, 213, 226), RGB(239, 244, 250));
					FillBox(dc, CRect(cx - 8, hy, cx + 8, hy + 1), RGB(41, 49, 61));
				}
			}

			const wchar_t * gridLabels[4] = {L"-10 ms",Tr(L"Temps ici"),L"+10 ms",L"Auto"};
			for (int control = 0; control < 4; ++control) {
				DrawControl(dc,L.gridControl[control],RGB(37,45,58),m_manualGrid[i] ? accent : kMuted);
				dc.SetTextColor(m_manualGrid[i] ? accent : kMuted);
				dj_skin::icon(dc,L.gridControl[control],control==0?dj_skin::Icon::left:control==1?dj_skin::Icon::anchor:control==2?dj_skin::Icon::right:dj_skin::Icon::automatic);
			}
			DrawControl(dc, L.key, m_fixedPitch[i] ? Blend(accent, kSurface, 0.84) : RGB(37,45,58), m_fixedPitch[i] ? accent : kMuted);
			dc.SetTextColor(m_fixedPitch[i] ? accent : kMuted);
			dj_skin::icon(dc,L.key,m_fixedPitch[i]?dj_skin::Icon::lock:dj_skin::Icon::unlock,m_fixedPitch[i]?accent:kText);

			DrawControl(dc, L.beat, Blend(accent, kSurface, 0.84), accent);
			dc.SetTextColor(accent);
			dj_skin::icon(dc,L.beat,dj_skin::Icon::beat);

			wchar_t rangeLabel[32]; swprintf_s(rangeLabel,L"\u00b1%d %%",m_speedRange[i].load());
            DrawControl(dc,L.range,RGB(37,45,58),accent);
            dc.SetTextColor(kText); dc.DrawText(rangeLabel,-1,&L.range,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            // Per-deck speed slider, neutral in the centre.
			{
				const auto area = L.speed; const double speed = m_speed[i].load();
				wchar_t label[64]; swprintf_s(label, Tr(L"Vitesse  %+.1f %%"), (speed-1.0)*100.0);
				CRect caption(L.range.right+4, area.top, L.beat.left - 5, area.top+28);
				DrawControl(dc,caption,RGB(8,10,14),RGB(65,72,82)); dc.SetTextColor(kText); std::wstring tempoLabel(label);
                double reference=m_cacheBpm[1-i]*m_speed[1-i].load();
                if(m_cacheLoaded[0]&&m_cacheLoaded[1]&&reference>0){
                    double ratio=m_cacheBpm[i]*speed/reference;
                    if(std::abs(ratio-.5)<1e-6)tempoLabel+=i==0?L" | ½×B":L" | ½×A";
                    if(std::abs(ratio-2)<1e-6)tempoLabel+=i==0?L" | 2×B":L" | 2×A";
                }
                dc.DrawText(tempoLabel.c_str(), -1, &caption, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
				const int left = area.left+10, right = area.right-10, y = area.bottom-8;
				if (right > left) {
					FillBox(dc, CRect(left,y-1,right,y+1), RGB(55,66,82));
					const int mid = (left+right)/2, x = left+(int)((right-left)*(speed-1+dj_tempo::range(m_speedRange[i].load()))/(2*dj_tempo::range(m_speedRange[i].load()))+0.5);
					FillBox(dc, CRect(mid,y-5,mid+1,y+5), kMuted);
					DrawControl(dc, CRect(x-4,y-5,x+5,y+6), accent, accent);
				}
			}

			DrawControl(dc, L.sync, Blend(accent, kSurface, 0.84), Blend(accent, kSurface, 0.35));
			dc.SetTextColor(accent);
			dj_skin::icon(dc,L.sync,dj_skin::Icon::bpm);

			// Boutons du bas
			DrawControl(dc, L.btn[0], playing ? accent : Blend(accent, kSurface, 0.83), accent);
			dc.SetTextColor(playing ? kCanvas : accent);
			dj_skin::icon(dc,L.btn[0],playing?dj_skin::Icon::pause:dj_skin::Icon::play,playing?accent:kText);
			DrawControl(dc, L.btn[1], RGB(37, 45, 58), RGB(55, 66, 82));
			dc.SetTextColor(kText);
			dj_skin::icon(dc,L.btn[1],dj_skin::Icon::start);
		}

		// Crossfader, en bas : une glissiere, un trait au milieu et un curseur.
		{
			const CRect strip = FaderStrip(rc);
			CRect shellA=strip,shellB=strip;shellA.right=(strip.left+strip.right)/2;shellB.left=shellA.right;
            dj_skin::surface(dc,shellA,DeckAccent(0),true);dj_skin::surface(dc,shellB,DeckAccent(1),true);
            CRect well=strip;well.DeflateRect(7,4);dj_skin::surface(dc,well,RGB(50,60,68));
			const int left = FaderLeft(rc), right = FaderRight(rc);
			const int cy = (strip.top + strip.bottom) / 2;
			const CRect both = PlayBothRect(rc);
			const bool ready = m_cacheLoaded[0] && m_cacheLoaded[1];
			DrawControl(dc, both, RGB(37,45,58), ready ? kMuted : RGB(55,66,82));
			dc.SetTextColor(ready ? kText : kMuted);
			CRect caption = both;
			const wchar_t * action = m_cachePlaying[0] && m_cachePlaying[1] ? L"II  Pause A + B" : (!m_cachePlaying[0] && !m_cachePlaying[1] ? Tr(L"\u25b6  Lancer A + B") : Tr(L"\u21c4  Inverser A / B"));
			dj_skin::icon(dc,caption,(m_alternate?(m_cachePlaying[0]||m_cachePlaying[1]):(m_cachePlaying[0]&&m_cachePlaying[1]))?dj_skin::Icon::pause:(!m_cachePlaying[0]&&!m_cachePlaying[1]?dj_skin::Icon::both:dj_skin::Icon::swap));

			CRect labelA(strip.left+68, strip.top, left - 4, strip.bottom);
			CRect labelB(right + 4, strip.top, right+24, strip.bottom);
			dc.SetTextColor(DeckAccent(0));
			dc.DrawText(L"A", -1, &labelA, DT_NOPREFIX | DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			dc.SetTextColor(DeckAccent(1));
			dc.DrawText(L"B", -1, &labelB, DT_NOPREFIX | DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			CRect track(left, cy - 1, right, cy + 1);
			dc.FillRect(&track, frame);
			const int midX = (left + right) / 2;
			CRect tick(midX - 1, cy - 8, midX + 1, cy + 8);
			dc.FillRect(&tick, frame);

			const int hx = left + (int)((right - left) * m_crossfade.load() + 0.5f);
			CRect handle(hx - 7, cy - 15, hx + 7, cy + 15);
			DrawControl(dc, handle, RGB(203, 213, 226), RGB(239, 244, 250));
			FillBox(dc, CRect(hx, cy - 10, hx + 1, cy + 10), kCanvas);
		}

        {auto meter=MasterMeterRect(rc);auto now=GetTickCount64();bool clipped=now<m_masterClipUntil.load(),limiting=m_limitEnabled.load()&&now<m_masterLimitUntil.load();
            COLORREF color=clipped?RGB(255,80,80):(limiting?RGB(255,182,60):(m_limitEnabled.load()?RGB(96,220,155):kMuted));
            DrawControl(dc,meter,kSurface,color);CFont meterFont;meterFont.CreatePointFont(70,L"Segoe UI");SelectObjectScope font(dc,(HGDIOBJ)(HFONT)meterFont);dc.SetTextColor(color);
            wchar_t label[32];if(clipped)wcscpy_s(label,L"CLIP");else if(limiting)swprintf_s(label,L"-%.1f dB",m_limitReduction.load());else wcscpy_s(label,m_limitEnabled.load()?L"LIM":L"LIM OFF");
            CRect text=meter;text.bottom-=7;dc.DrawText(label,-1,&text,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
            CRect bar=meter;bar.DeflateRect(5,3);bar.top=bar.bottom-3;FillBox(dc,bar,RGB(25,30,36));double peak=m_masterPeak.load();double level=peak>0?std::clamp((20.*std::log10(peak)+60.)/60.,0.,1.):0.;bar.right=bar.left+int(bar.Width()*level);FillBox(dc,bar,color);
        }

		// Shared settings; changing appearance/language does not reload audio.
		{
            CRect button=LanguageRect(rc,false);
            DrawControl(dc,button,kSurface,RGB(65,72,82));
            dj_skin::icon(dc,button,dj_skin::Icon::settings);
		}

		// Numero de version, en petit et en discret, en bas a droite
		{
			CFont smallFont;
			smallFont.CreatePointFont(70, L"Segoe UI");
			SelectObjectScope smallFontScope(dc, (HGDIOBJ)(HFONT)smallFont);
			dc.SetTextColor(Blend(textColor, bgColor, 0.5));
			CRect versionRect = VersionRect(rc);
			dc.DrawText(L"foo_dj v" FOO_DJ_VERSION_W, -1, &versionRect, DT_NOPREFIX | DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
	}

	// ---------------------------------------------------------------- souris

	void CDjPanel::OnLButtonDown(UINT nFlags, CPoint point) {
		CRect rc;
		GetClientRect(&rc);

        m_pressedButton=HitButton(point,rc);
        if(!m_pressedButton.IsRectEmpty()) {m_hotButton=m_pressedButton;SetCapture();InvalidateButton(m_pressedButton);UpdateWindow();}
        if(LanguageRect(rc,false).PtInRect(point)){m_pressedButton.SetRectEmpty();::ReleaseCapture();OpenSettings();Invalidate();return;}

		if (PlayBothRect(rc).PtInRect(point)) { PlayBoth(); Invalidate(); return; }

		// Crossfader : on attrape le curseur
		if (CRect(FaderLeft(rc)-10,FaderStrip(rc).top,FaderRight(rc)+10,FaderStrip(rc).bottom).PtInRect(point)) {
			m_drag = 1;
			SetCapture();
			SetFaderFromX(point.x, rc);
			return;
		}

		for (int i = 0; i < 2; ++i) {
			const DeckLayout L = MakeLayout(i, rc);

			if(L.loopSnap.PtInRect(point)){SetLoopSnap(i,!m_loopSnap[i]);return;}
            for(int k=0;k<4;++k)if(L.loop[k].PtInRect(point)){EditLoop(i,k);return;}
            if(L.stemPrepare.PtInRect(point)){StemsMenu(i,point);return;}
            for(int k=0;k<4;++k){
                if(L.stemSolo[k].PtInRect(point)){SoloStem(i,k);return;}
                if(L.stemVolume[k].PtInRect(point)){m_drag=10+i*4+k;SetCapture();SetStemVolumeFromX(i,k,point.x,rc);return;}
            }
            for(int k=0;k<4;++k)if(L.stem[k].PtInRect(point)){ToggleStem(i,k);return;}
            if(L.automation.PtInRect(point)) { AutomationMenu(i,point);return; }
            if(L.queueOpen.PtInRect(point) || L.queue[0].PtInRect(point) || L.queue[1].PtInRect(point)) { OpenQueue(i);return; }
            if(L.release.PtInRect(point)) { if(m_pendingLoad[i]||!m_savedPath[i].empty()) ReleaseForTags(i); Invalidate();return; }
            if(L.headphones.PtInRect(point)){
                if(cfg_cueBackend.get()==GUID{}){m_pressedButton.SetRectEmpty();::ReleaseCapture();OpenSettings(2);}
                else{StopPreview();m_cue[i]=!m_cue[i].load();ConfigureCue();}
                Invalidate();return;
            }
            if (L.btn[0].PtInRect(point)) { TogglePlay(i); Invalidate(); return; }
			if (L.btn[1].PtInRect(point)) { SeekDeck(i, 0.0); Invalidate(); return; }
			if (L.zoomOut.PtInRect(point)) { ZoomDeck(i, false); return; }
			if (L.zoomIn.PtInRect(point)) { ZoomDeck(i, true); return; }
			if (L.wave.PtInRect(point)) { SeekFromWaveX(i, point.x, rc); Invalidate(); return; }
			for (int control = 0; control < 4; ++control) if (L.gridControl[control].PtInRect(point)) { EditGrid(i,control); Invalidate(); return; }
			if (L.beat.PtInRect(point)) { SyncBeats(i); Invalidate(); return; }
			for(int k=0;k<3;++k) if(L.analyse[k].PtInRect(point)) { AnalyseDeck(i,k); return; }
            if(L.matchKey.PtInRect(point)) { MatchKey(i); return; }
            for(int k=0;k<3;++k) if(L.pitch[k].PtInRect(point)) {
                m_transpose[i]=k==1?0:(std::max)(-12,(std::min)(12,m_transpose[i].load()+(k==0?-1:1)));
                SaveSession(); Invalidate(); return;
            }
            if (L.key.PtInRect(point)) { m_fixedPitch[i] = !m_fixedPitch[i].load(); SaveSession(); Invalidate(); return; }
			if (L.sync.PtInRect(point)) { MatchTempo(i); Invalidate(); return; }
			if(L.range.PtInRect(point)) {
                HMENU menu=CreatePopupMenu();
                for(int value:{8,16,50}) {
                    wchar_t text[24]; swprintf_s(text,L"\u00b1%d %%",value);
                    AppendMenuW(menu,MF_STRING|(m_speedRange[i]==value?MF_CHECKED:0),value,text);
                }
                CPoint screen(L.range.left,L.range.bottom); ClientToScreen(&screen);
                int selected=(int)TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY,screen.x,screen.y,0,m_hWnd,nullptr);
                DestroyMenu(menu);
                if(selected==8 || selected==16 || selected==50) {
                    m_speedRange[i]=selected; m_speed[i]=dj_tempo::clamp(m_speed[i].load(),selected);
                    m_error[i].clear(); SaveSession(); Invalidate();
                }
                return;
            }
            if (L.speed.PtInRect(point)) { m_drag = 4 + i; SetCapture(); SetSpeedFromX(i, point.x, rc); return; }
			if (L.vol.PtInRect(point)) {              // volume : on attrape le curseur
				m_drag = 2 + i;
				SetCapture();
				SetVolumeFromY(i, point.y, rc);
				return;
			}
		}
	}

	void CDjPanel::OnMouseMove(UINT nFlags, CPoint point) {
        CRect client;GetClientRect(&client);
        if(!m_trackingMouse) { TRACKMOUSEEVENT event={sizeof(event),TME_LEAVE,m_hWnd,0};::TrackMouseEvent(&event);m_trackingMouse=true; }
        auto hot=HitButton(point,client);
        if(hot!=m_hotButton) {auto old=m_hotButton;m_hotButton=hot;InvalidateButton(old);InvalidateButton(hot);}
		if (m_drag == 0) return;
		if (!(nFlags & MK_LBUTTON)) {   // le bouton a ete relache ailleurs
			m_drag = 0;
			return;
		}
		CRect rc;
		GetClientRect(&rc);
		if(m_drag>=10){int index=m_drag-10;SetStemVolumeFromX(index/4,index%4,point.x,rc);}
        else if (m_drag == 1) SetFaderFromX(point.x, rc);
		else if (m_drag >= 4) SetSpeedFromX(m_drag - 4, point.x, rc);
		else SetVolumeFromY(m_drag - 2, point.y, rc);
	}

	void CDjPanel::OnLButtonUp(UINT nFlags, CPoint point) {
        if(!m_pressedButton.IsRectEmpty()) {auto old=m_pressedButton;m_pressedButton.SetRectEmpty();::ReleaseCapture();InvalidateButton(old);}
		if (m_drag != 0) {
			m_drag = 0;
			::ReleaseCapture();
		}
	}

	// Clic droit sur un curseur : retour a sa valeur d'origine (crossfader au milieu, volume au maximum).
	void CDjPanel::OnRButtonDown(UINT nFlags, CPoint point) {
		CRect rc;
		GetClientRect(&rc);
		if (CRect(FaderLeft(rc)-10,FaderStrip(rc).top,FaderRight(rc)+10,FaderStrip(rc).bottom).PtInRect(point)) {
			CancelAutomation(); m_crossfade = 0.5f;
			Invalidate();
			return;
		}
		for (int i = 0; i < 2; ++i) {
			for(int k=0;k<4;++k)if(MakeLayout(i,rc).stemVolume[k].PtInRect(point)){SetStemVolume(i,k,1);return;}
            if (MakeLayout(i, rc).speed.PtInRect(point)) { m_speed[i] = 1.0; Invalidate(); return; }
			if (MakeLayout(i, rc).vol.PtInRect(point)) {
				m_volume[i] = 1.0f;
				Invalidate();
				return;
			}
		}
	}

	// Molette sur une forme d'onde : zoom avant (vers le haut) ou arriere (vers le bas).
	BOOL CDjPanel::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
		ScreenToClient(&pt);
		CRect rc;
		GetClientRect(&rc);
		for (int i = 0; i < 2; ++i) {
			const auto layout = MakeLayout(i,rc);
            for(int k=0;k<4;++k)if(layout.stemVolume[k].PtInRect(pt)){SetStemVolume(i,k,m_stemVolume[i][k]+(zDelta/(float)WHEEL_DELTA)*((nFlags&MK_SHIFT)?.01f:.05f));return TRUE;}
			CRect gridRow(layout.gridControl[0].left,layout.gridControl[0].top,layout.gridControl[3].right,layout.gridControl[3].bottom);
			if (gridRow.PtInRect(pt)) { EditGrid(i,zDelta < 0 ? 0 : 2, ((nFlags & MK_SHIFT) ? 0.001 : 0.01) * std::abs((double)zDelta) / WHEEL_DELTA); Invalidate(); return TRUE; }
			if (MakeLayout(i, rc).speed.PtInRect(pt)) {
				const double step = (nFlags & MK_SHIFT) ? 0.0001 : 0.001;
				m_speed[i] = dj_tempo::clamp(m_speed[i].load() + (double)zDelta / WHEEL_DELTA * step,m_speedRange[i].load());
				Invalidate(); return TRUE;
			}
			if (MakeLayout(i, rc).wave.PtInRect(pt)) {
				ZoomDeck(i, zDelta > 0);
				return TRUE;
			}
		}
		SetMsgHandled(FALSE);
		return FALSE;
	}

	// ---------------------------------------------------------------- chargement et lecture

	// Arrete l'analyse de forme d'onde en cours pour ce deck (s'il y en a une).
	void CDjPanel::StopWaveThread(int deck) {
		if (m_waveCur[deck]) m_waveCur[deck]->abort.abort();
		if (m_waveThread[deck].joinable()) m_waveThread[deck].join();
		m_waveCur[deck].reset();
	}

	// Ouvre un morceau (adresse foobar2000 + numero de sous-piste) avec le decodeur de foobar2000,
	// lit sa duree et decode un premier bloc (qui sert aussi a connaitre le format). Le deck est
	// ensuite pret a jouer, et la forme d'onde se calcule en arriere-plan.
    void CDjPanel::RefreshDeckTags(int deck, bool readFile) {
        if(m_savedPath[deck].empty()) return;
        auto handle=metadb::get()->handle_create(m_savedPath[deck].c_str(),m_savedSubsong[deck]);
        file_info_impl info;
        const bool haveDatabase=handle->get_info(info);
        file_info_impl diskInfo;
        bool haveDisk=false;
        if(readFile) {
            try { abort_callback_dummy abort; input_helper::g_get_info(handle->get_location(),diskInfo,abort); haveDisk=true; }
            catch(const std::exception& e) { popup_message::g_show(e.what(),"DJ Decks - tags"); return; }
        }
        if(!haveDatabase && !haveDisk) {
            if(readFile) popup_message::g_show("No metadata available.","DJ Decks");
            return;
        }
        std::wstring key,bpmText; double bpm=0;
        for(const char* field:{"INITIALKEY","INITIAL KEY","KEY"}) {
            auto tag=info.meta_get(field,0);
            if((!tag || !*tag) && haveDisk) tag=diskInfo.meta_get(field,0);
            if(tag) key=dj_key::display(pfc::stringcvt::string_wide_from_utf8(tag).get_ptr());
            if(!key.empty()) break;
        }
        auto tag=haveDisk?diskInfo.meta_get("BPM",0):nullptr;
        if((!tag || !*tag) && haveDisk) tag=diskInfo.meta_get("TEMPO",0);
        if(!tag || !*tag) tag=info.meta_get("BPM",0);
        if(!tag || !*tag) tag=info.meta_get("TEMPO",0);
        if(tag) {
            std::string value=tag;
            bpmText=pfc::stringcvt::string_wide_from_utf8(tag).get_ptr();
            std::replace(value.begin(),value.end(),',','.');
            std::istringstream parser(value); parser.imbue(std::locale::classic());
            if(!(parser>>bpm) || !std::isfinite(bpm) || bpm<=0) bpm=0;
        }
        { std::lock_guard<std::mutex> lock(m_lock);
          if(!m_deck[deck].loaded) return;
          m_deck[deck].musicalKey=key; m_deck[deck].bpm=bpmText; m_deck[deck].bpmValue=bpm;
          if(readFile) m_error[deck].clear();
        }
        UpdateReplayGain(deck,haveDisk?static_cast<const file_info&>(diskInfo):static_cast<const file_info&>(info));
        m_gridStamp[deck]=0; Invalidate();
        if(readFile) {
            wchar_t values[256];
            const auto keyLabel=dj_key::label(key);
            swprintf_s(values,g_english.load()?L"Tags read.\nTagged BPM: %.2f\nEffective BPM (speed applied): %.2f\nKey: %s\n\nThis button reads tags; it does not analyse audio.":L"Tags relus.\nBPM du tag : %.2f\nBPM effectif (vitesse appliqu\u00e9e) : %.2f\nTonalit\u00e9 : %s\n\nCe bouton relit les tags ; il ne calcule pas le BPM.",bpm,bpm*m_speed[deck].load(),keyLabel.empty()?L"?":keyLabel.c_str());
            MessageBoxW(values,L"DJ Decks",MB_OK|MB_ICONINFORMATION);
        }
    }
    void CDjPanel::MatchKey(int deck) {
        if(m_pendingLoad[deck])return;
        const wchar_t* error=nullptr;
        { std::lock_guard<std::mutex> lock(m_lock);
          if(!m_deck[deck].loaded || !m_deck[1-deck].loaded)
            error=g_english.load()?L"Load both decks first.":L"Chargez les deux platines.";
          else if((!m_fixedPitch[0].load() && std::abs(m_speed[0].load()-1)>0.000001)
               || (!m_fixedPitch[1].load() && std::abs(m_speed[1].load()-1)>0.000001))
            error=g_english.load()?L"Enable key lock on decks with non-neutral vinyl speed first.":L"Activez la tonalit\u00e9 fixe sur les platines dont la vitesse vinyle n'est pas neutre.";
          else {
            int shift=0;
            if(dj_key::match(m_deck[deck].musicalKey,m_deck[1-deck].musicalKey,m_transpose[deck].load(),m_transpose[1-deck].load(),shift))
                m_transpose[deck]=shift;
            else error=g_english.load()?L"Two recognized musical key tags are required.":L"Deux tags de tonalit\u00e9 reconnus sont requis.";
          }
        }
        if(error) { MessageBoxW(error,L"DJ Decks",MB_OK|MB_ICONINFORMATION); return; }
        SaveSession(); Invalidate();
    }

    // The external analysis command is asynchronous and has no shared completion API.
    // Keep all readers of this physical file released until the user explicitly plays.
    void CDjPanel::ReleaseForTags(int deck) {
        ClearPreparations();m_prepareSuspended[0]=m_prepareSuspended[1]=true;for(int i=0;i<2;++i){m_pendingLoad[i]=m_pendingPlay[i]=false;m_pendingSeek[i]=-1;}
        StopPreview(true);
        CancelAutomation();
        if(m_stemOwner>=0 && playable_location::path_compare(m_savedPath[m_stemOwner].c_str(),m_savedPath[deck].c_str())==0)CancelStems();
        const auto path=m_savedPath[deck];
        for(int i=0;i<2;++i) if(playable_location::path_compare(path.c_str(),m_savedPath[i].c_str())==0) {
            if(m_deck[i].wave) {
                std::lock_guard<std::mutex> waveLock(m_deck[i].wave->m);
                m_waveNeedsRebuild[i] = m_waveNeedsRebuild[i] || !m_deck[i].wave->done;
            }
            { std::lock_guard<std::mutex> lock(m_lock); ReleaseDeckReader(m_deck[i]); m_stemActive[i]=false; m_error[i].clear(); }
            StopWaveThread(i); // closes the waveform/artwork reader too

        }
        SaveSession(); Invalidate();
    }
    bool CDjPanel::ReopenAfterTags(int deck) {
        { std::lock_guard<std::mutex> lock(m_lock);
          if(!m_deck[deck].loaded || m_deck[deck].helper) return true;
        }
        try {
            abort_callback_dummy abort;
            auto helper=std::make_shared<input_helper>();
            auto location=make_playable_location(m_savedPath[deck].c_str(),m_savedSubsong[deck]);
            helper->open(service_ptr_t<file>(),location,input_flag_no_looping,abort);
            double position=0;
            { std::lock_guard<std::mutex> lock(m_lock); position=m_outRate?m_deck[deck].framesPlayed/m_outRate:0; }
            if(position>0) {
                if(!helper->can_seek()) throw pfc::exception("Cannot restore position after tag analysis");
                helper->seek(position,abort);
            }
            { std::lock_guard<std::mutex> lock(m_lock);
              auto& d=m_deck[deck]; d.helper=helper; d.eof=false; d.rate=d.sourceRate;d.channels=d.sourceChannels;
              d.src.clear(); d.srcPos=0; d.pending.clear(); d.pendingPos=0;
              d.stretch.reset(); d.stretchFlushed=false; m_error[deck].clear();
            }
            RefreshDeckTags(deck);
            // If waveform analysis was interrupted, rebuild it only after explicit resume.
            if(m_waveNeedsRebuild[deck]) {
                auto wave=std::make_shared<Waveform>();
                { std::lock_guard<std::mutex> lock(m_lock); m_deck[deck].wave=wave; }
                m_waveNeedsRebuild[deck]=false; m_waveCur[deck]=wave;
                m_waveThread[deck]=std::thread(AnalyzeWaveform,m_savedPath[deck],m_savedSubsong[deck],wave);
            }
            return true;
        } catch(const std::exception& e) {
            popup_message::g_show(e.what(),"DJ Decks - finish tag writing before resuming"); return false;
        }
    }

    void CDjPanel::AnalyseDeck(int deck,int action) {
        if(m_pendingLoad[deck])return;
        if(m_savedPath[deck].empty()) {
            popup_message::g_show(g_english.load()?"Load a track first.":"Chargez d'abord un morceau.","DJ Decks"); return;
        }
        if(action==2) { RefreshDeckTags(deck,true); return; }
        // Public context-command identifiers from the components' upstream sources.
        static const GUID bpm={0x518d0234,0x3134,0x4fea,{0xbd,0x43,0x8f,0x61,0xee,0x3a,0x35,0x3b}};
        static const GUID smart={0x51fa6f82,0x7669,0x4090,{0x99,0x7c,0x85,0x07,0xc6,0xec,0x2e,0x30}};
        static const GUID key={0xefb0d751,0x1a4e,0x4ea3,{0x9d,0x22,0x68,0x25,0x64,0xe7,0xad,0xca}};
        metadb_handle_list tracks;
        tracks.add_item(metadb::get()->handle_create(m_savedPath[deck].c_str(),m_savedSubsong[deck]));
        bool hasSmart=false,hasClassic=false,hasKey=false;
        for(auto item:service_enum_t<contextmenu_item>())for(unsigned n=0;n<item->get_num_items();++n){auto id=item->get_item_guid(n);hasSmart=hasSmart||id==smart;hasClassic=hasClassic||id==bpm;hasKey=hasKey||id==key;}
        int provider=dj_bpm::select(cfg_bpmProvider.get(),hasSmart,hasClassic);
        if((action==0&&provider<0)||(action!=0&&!hasKey)){popup_message::g_show(g_english.load()?"Selected analyser unavailable. Install foo_smart_tempo / foo_bpm for BPM, or foo_musical_key for key; check Settings > Audio.":"Analyseur choisi indisponible. Installer foo_smart_tempo / foo_bpm pour le BPM, ou foo_musical_key pour la tonalité ; vérifier Réglages > Audio.","DJ Decks");return;}
        const GUID command=action==0?(provider==1?smart:bpm):key;
        ReleaseForTags(deck);
        // Never hold the audio mutex while another component opens a modal dialog.
        try {
            if(!menu_helpers::run_command_context(command,pfc::guid_null,tracks))
                popup_message::g_show(g_english.load()?"Analysis command unavailable. Check foo_smart_tempo / foo_bpm / foo_musical_key installation.":"Commande indisponible. Verifiez l'installation de foo_smart_tempo / foo_bpm / foo_musical_key.","DJ Decks");
        } catch(const std::exception& e) { popup_message::g_show(e.what(),"DJ Decks"); }
        RefreshDeckTags(deck);
    }

    void CDjPanel::LoadDeck(int deck,const char* path,t_uint32 subsong){
        if(m_applyImport[deck]&&(m_importDeck[deck].item.path!=path||m_importDeck[deck].item.subsong!=subsong))m_applyImport[deck]=false;
        m_prepareSuspended[deck]=false;dj_queue::Item item{path,subsong};auto prepared=TakePreparation(deck,item);
        if(!TrimOptions().enabled()||prepared){m_pendingLoad[deck]=false;CommitLoadDeck(deck,path,subsong,prepared);return;}
        if(!m_autoChanging)CancelAutomation();
        m_pendingLoad[deck]=true;m_pendingPlay[deck]=false;m_pendingSeek[deck]=-1;m_pendingItem[deck]=item;
        {std::lock_guard<std::mutex> lock(m_lock);m_deck[deck].playing=false;m_error[deck]=g_english.load()?L"Preparing silence trim…":L"Préparation des silences…";}
        RequestPreparation(deck,item);Invalidate();
    }
    void CDjPanel::PreparationTick(){
        for(int i=0;i<2;++i){
            if(m_pendingLoad[i]){auto ready=TakePreparation(i,m_pendingItem[i]);if(!ready)continue;
                auto item=m_pendingItem[i];bool play=m_pendingPlay[i];double seek=m_pendingSeek[i];m_pendingLoad[i]=m_pendingPlay[i]=false;m_pendingSeek[i]=-1;
                CommitLoadDeck(i,item.path.c_str(),item.subsong,ready);if(seek>=0)SeekDeck(i,seek);
                if(play){std::lock_guard<std::mutex> lock(m_lock);if(m_deck[i].loaded&&m_deck[i].helper&&m_error[i].empty()){m_deck[i].playing=true;EnsureEngine();}}
            }else if(!m_prepareSuspended[i]&&TrimOptions().enabled()&&(m_chain[i]||m_alternate)){
                {std::lock_guard<std::mutex> lock(m_lock);if(!m_deck[i].loaded||!m_deck[i].helper||!m_error[i].empty())continue;}
                int next=m_queueCurrent[i]+1;if(next>=0&&next<(int)m_queue[i].size())RequestPreparation(i,m_queue[i][next]);else DropPreparation(i);
            }else DropPreparation(i);
        }
    }
    void CDjPanel::CommitLoadDeck(int deck,const char* path,t_uint32 subsong,std::shared_ptr<PreparedTrack> prepared) {
        const auto imported=m_importDeck[deck];bool applyImport=m_applyImport[deck]&&imported.item.path==path&&imported.item.subsong==subsong;m_applyImport[deck]=false;
        if(!m_autoChanging)CancelAutomation();
        m_autoArmed[deck]=false;
        if(m_stemOwner==deck)CancelStems();
        m_stemReady[deck]=m_stemActive[deck]=false;m_stemMask[deck]=15;m_stemSolo[deck]=0;m_stemSolo[deck]=0;for(auto& v:m_stemVolume[deck])v=1;m_stemKey[deck].clear();
		StopWaveThread(deck);   // l'ancienne analyse n'a plus d'interet
		m_loopNotice[deck].clear();
        m_view[deck] = 0.0;     // on repart en vue complete
        m_waveNeedsRebuild[deck]=false;

		// Nom affiche : la partie apres le dernier "\" ou "/" de l'adresse
		{
			const char * nameStart = path;
			for (const char * q = path; *q; ++q) {
				if (*q == '\\' || *q == '/') nameStart = q + 1;
			}
			pfc::stringcvt::string_wide_from_utf8 name(nameStart);
			m_name[deck] = name.get_ptr();
			m_artist[deck].clear();
		}

		try {
			playable_location_impl location;
			location.set_path(path);
			location.set_subsong(subsong);

			abort_callback_dummy abort;

			Deck nd;
            if(prepared&&!prepared->error.empty())throw std::runtime_error(prepared->error);
            if(prepared)nd=std::move(prepared->deck);
            else nd.helper = std::make_shared<input_helper>();
			// Pas de "simpledecode" ici : il interdit le positionnement dans le fichier, dont on a besoin.
            if(!prepared)nd.helper->open(service_ptr_t<file>(), location, input_flag_no_looping, abort);

			file_info_impl info;
            if(prepared)info.copy(prepared->info);else nd.helper->get_info(subsong, info, abort);
			nd.length = info.get_length();

            // Key analysers can store metadata in foobar's DB without writing the file.
            // Prefer the host's current metadata, falling back to decoder file tags.
            file_info_impl libraryInfo;
            auto handle = metadb::get()->handle_create(location);
            const bool haveLibraryInfo = handle->get_info(libraryInfo);
            for (int source = 0; source < 2 && nd.musicalKey.empty(); ++source) {
                if (source == 0 && !haveLibraryInfo) continue;
                const file_info& keyInfo = source == 0 ? static_cast<const file_info&>(libraryInfo) : static_cast<const file_info&>(info);
                for (const char* field : {"INITIALKEY", "INITIAL KEY", "KEY"}) {
                    const char* tag = keyInfo.meta_get(field, 0);
                    if (!tag) continue;
                    nd.musicalKey = dj_key::display(pfc::stringcvt::string_wide_from_utf8(tag).get_ptr());
                    if (!nd.musicalKey.empty()) break;
                }
            }
			const char * bpmTag = info.meta_get("bpm", 0);
			if (!bpmTag || !*bpmTag) bpmTag = info.meta_get("tempo", 0);
			if (bpmTag && *bpmTag) {
				std::string value = bpmTag;
				const size_t first = value.find_first_not_of(" \t\r\n");
				if (first != std::string::npos) {
					value = value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
					nd.bpm = pfc::stringcvt::string_wide_from_utf8(value.c_str()).get_ptr();
					std::replace(value.begin(), value.end(), ',', '.');
					std::istringstream parser(value); parser.imbue(std::locale::classic());
					double bpm = 0; if ((parser >> bpm) && std::isfinite(bpm) && bpm > 0) nd.bpmValue = bpm;
				}
			}
			// Track tags, including the actual subsong, take precedence over the filename.
			const char * titleTag = info.meta_get("title", 0);
			const char * artistTag = info.meta_get("artist", 0);
			if (!artistTag || !*artistTag) artistTag = info.meta_get("album artist", 0);
			std::wstring display = m_name[deck];
			if (titleTag && *titleTag) display = pfc::stringcvt::string_wide_from_utf8(titleTag).get_ptr();
			if (artistTag && *artistTag) {
				m_artist[deck] = pfc::stringcvt::string_wide_from_utf8(artistTag).get_ptr();
			}
			m_name[deck] = std::move(display);

            if(!prepared){audio_chunk_impl chunk;
                if(!nd.helper->run(chunk,abort))throw pfc::exception("Aucun son decode");
                nd.rate=chunk.get_sample_rate();nd.channels=chunk.get_channel_count();nd.sourceRate=nd.rate;nd.sourceChannels=nd.channels;AppendStereo(nd,chunk);
            }
            nd.framesPlayed=nd.trimStart*(m_outRate?m_outRate:48000);
			nd.wave = std::make_shared<Waveform>();
			nd.loaded = true;

			std::shared_ptr<Waveform> wave = nd.wave;
			{
				std::lock_guard<std::mutex> lock(m_lock);
				nd.loop=dj_loop::restore(m_loopLibrary,GridKey(path,subsong),nd.trimStart,nd.trimEnd>0?nd.trimEnd:nd.length);
                if(applyImport)nd.loop=dj_session::bounded(imported.loop,nd.trimStart,nd.trimEnd>0?nd.trimEnd:nd.length);
                m_deck[deck] = std::move(nd);    // remplace l'ancien contenu (et l'arrete)
				m_error[deck].clear();
			}

			m_savedPath[deck] = path;
			m_savedSubsong[deck] = subsong;
            UpdateReplayGain(deck,haveLibraryInfo?static_cast<const file_info&>(libraryInfo):static_cast<const file_info&>(info));
            CheckStems(deck);
			m_manualGrid[deck] = false; m_manualPhase[deck] = 0;
			const auto savedGrid = m_gridLibrary.find(GridKey(m_savedPath[deck],subsong));
			if (savedGrid != m_gridLibrary.end() && savedGrid->second.is<double>()) {
				const auto grid = dj_beats::manual(m_deck[deck].bpmValue,savedGrid->second.get<double>());
				if (grid.valid) { m_manualGrid[deck] = true; m_manualPhase[deck] = grid.offset; }
			}
			if(applyImport){m_manualGrid[deck]=false;m_manualPhase[deck]=0;if(imported.manual){auto grid=dj_beats::manual(m_deck[deck].bpmValue,imported.phase);if(grid.valid){m_manualGrid[deck]=true;m_manualPhase[deck]=grid.offset;}}}
            if (!m_restoring) SaveSession();

			// Calcul de la forme d'onde en arriere-plan
			m_waveCur[deck] = wave;
			m_waveThread[deck] = std::thread(AnalyzeWaveform, std::string(path), subsong, wave);
		} catch (const std::exception & e) {
			pfc::stringcvt::string_wide_from_utf8 message(e.what());
			std::lock_guard<std::mutex> lock(m_lock);
			m_deck[deck] = Deck();
			m_error[deck] = std::wstring(L"Erreur : ") + message.get_ptr();
		}
	}



	static std::wstring SessionFile(const char * name = "foo_dj-session.json") {
		const auto path = filesystem::g_get_native_path(core_api::pathInProfile(name));
		return pfc::stringcvt::string_wide_from_utf8(path).get_ptr();
	}
	static std::string ReadSessionFile(const char * name = "foo_dj-session.json") {
		const auto path = SessionFile(name);
		HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE) return {};
		const DWORD size = GetFileSize(file, nullptr);
		std::string text;
		if (size != INVALID_FILE_SIZE && size <= 4 * 1024 * 1024) {
			text.resize(size); DWORD read = 0;
			if (!ReadFile(file, text.data(), size, &read, nullptr) || read != size) text.clear();
		}
		CloseHandle(file); return text;
	}
	static bool WriteSessionFile(const std::string & text, const char * name = "foo_dj-session.json") {
		const auto path = SessionFile(name), temp = path + L".tmp";
		HANDLE file = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE) return false;
		DWORD written = 0;
		const bool ok = WriteFile(file, text.data(), (DWORD)text.size(), &written, nullptr) && written == text.size() && FlushFileBuffers(file);
		CloseHandle(file);
		return ok && MoveFileExW(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}


	void CDjPanel::SaveSession() {
		if (m_restoring) return;
		picojson::array items;
		for (int i = 0; i < 2; ++i) {
			picojson::object item;
			item["autoChain"]=picojson::value(m_chain[i]);
            item["fadeSeconds"]=picojson::value(m_fadeSeconds);
            item["linearFade"]=picojson::value(m_linearFade.load());
            // Alternation is deliberately not armed on restart: explicit opt-in each session.
            item["queue"] = dj_queue::encode(m_queue[i]);
            item["queueCurrent"] = picojson::value(double(m_queueCurrent[i]));
			item["path"] = picojson::value(m_pendingLoad[i]?m_pendingItem[i].path:m_savedPath[i]);
			item["subsong"] = picojson::value((double)(m_pendingLoad[i]?m_pendingItem[i].subsong:m_savedSubsong[i]));
			item["fixedPitch"] = picojson::value(m_fixedPitch[i].load());
			item["speed"] = picojson::value(m_speed[i].load());
            item["speedRange"] = picojson::value((double)m_speedRange[i].load());
            item["transpose"] = picojson::value((double)m_transpose[i].load());
			// Only the position snapshot needs the audio lock; JSON and disk writes do not.
            double position=0;
            {std::lock_guard<std::mutex> lock(m_lock);position=(double)m_deck[i].framesPlayed/(m_outRate?m_outRate:48000);}
            if(m_pendingLoad[i])position=(std::max)(0.,m_pendingSeek[i]);
            item["position"] = picojson::value(position);
			items.push_back(picojson::value(item));
		}
		const std::string text = picojson::value(items).serialize();
		cfg_session = text.c_str();
		try {
			if (!WriteSessionFile(text)) FB2K_console_formatter() << "foo_dj : session file could not be saved";
			if (!WriteSessionFile(picojson::value(m_gridLibrary).serialize(),"foo_dj-grids.json")) FB2K_console_formatter() << "foo_dj : beat grids could not be saved";
            if (!WriteSessionFile(picojson::value(m_loopLibrary).serialize(),"foo_dj-loops.json")) FB2K_console_formatter() << "foo_dj : A/B loop points could not be saved";
		} catch (const std::exception & e) { FB2K_console_formatter() << "foo_dj : session save: " << e.what(); }
	}
    dj_session::Bundle CDjPanel::CaptureBundle(bool full){
        if(full&&(m_pendingLoad[0]||m_pendingLoad[1]))throw std::runtime_error("Wait until deck loading completes / Attendez la fin du chargement");
        dj_session::Bundle out;out.full=full;
        for(int i=0;i<2;++i){out.lists[i].items=m_queue[i];out.lists[i].current=m_queueCurrent[i];}
        if(!full)return out;
        {std::lock_guard<std::mutex> lock(m_lock);for(int i=0;i<2;++i){auto& saved=out.decks[i];auto& deck=m_deck[i];saved.item={m_savedPath[i],m_savedSubsong[i]};saved.position=deck.framesPlayed/(m_outRate?m_outRate:48000);saved.loop=deck.loop;saved.loop.enabled=false;saved.speed=m_speed[i].load();saved.range=m_speedRange[i].load();saved.fixed=m_fixedPitch[i].load();saved.transpose=m_transpose[i].load();saved.volume=m_volume[i].load();saved.snap=m_loopSnap[i];saved.manual=m_manualGrid[i];saved.phase=m_manualPhase[i];}
         out.crossfade=m_crossfade.load();out.fade=m_fadeSeconds;out.linear=m_linearFade.load();}
        std::map<std::string,dj_queue::Item> referenced;for(auto& list:out.lists)for(auto& item:list.items)referenced[GridKey(item.path,item.subsong)]=item;for(auto& deck:out.decks)if(!deck.item.path.empty())referenced[GridKey(deck.item.path,deck.item.subsong)]=deck.item;
        for(auto& pair:referenced){dj_session::Marker marker;marker.item=pair.second;marker.loop=dj_loop::restore(m_loopLibrary,pair.first,0,0);auto grid=m_gridLibrary.find(pair.first);if(grid!=m_gridLibrary.end()&&grid->second.is<double>()){marker.manual=true;marker.phase=grid->second.get<double>();}out.markers.push_back(marker);}return out;
    }
    void CDjPanel::ApplyBundle(const dj_session::Bundle& input,bool restoreDecks){
        // Validate the complete document before touching any live state.
        const auto bundle=dj_session::decode(dj_session::encode(input));
        if(m_pendingLoad[0]||m_pendingLoad[1])throw std::runtime_error("Wait until deck loading completes / Attendez la fin du chargement");
        bool full=restoreDecks&&bundle.full;
        auto nextLoops=m_loopLibrary,nextGrids=m_gridLibrary;
        if(full){
            auto remember=[&](const dj_queue::Item& item,const dj_loop::Region& loop,bool manual,double phase){if(item.path.empty())return;auto key=GridKey(item.path,item.subsong);dj_loop::remember(nextLoops,key,loop);if(manual)nextGrids[key]=picojson::value(phase);else nextGrids.erase(key);};
            for(auto& marker:bundle.markers)remember(marker.item,marker.loop,marker.manual,marker.phase);
            for(auto& deck:bundle.decks)remember(deck.item,deck.loop,deck.manual,deck.phase);
            if(picojson::value(nextLoops).serialize().size()>4*1024*1024||picojson::value(nextGrids).serialize().size()>4*1024*1024)throw std::runtime_error("Point/grid library limit exceeded; session unchanged / Bibliothèque de repères trop volumineuse ; session conservée");
        }
        CancelAutomation();m_chain[0]=m_chain[1]=false;StopPreview();
        if(full){
            ClearPreparations();CancelStems();for(int i=0;i<2;++i)StopWaveThread(i);
            {std::lock_guard<std::mutex> lock(m_lock);m_autoFade={};for(int i=0;i<2;++i)m_deck[i]=Deck{};}
            m_cue[0]=m_cue[1]=false;ConfigureCue();
            for(int i=0;i<2;++i){m_applyImport[i]=false;m_pendingLoad[i]=m_pendingPlay[i]=false;m_pendingSeek[i]=-1;m_prepareSuspended[i]=false;m_savedPath[i].clear();m_savedSubsong[i]=0;m_name[i].clear();m_artist[i].clear();m_error[i].clear();m_loopNotice[i].clear();m_manualGrid[i]=false;m_manualPhase[i]=0;m_stemReady[i]=m_stemActive[i]=false;m_gridStamp[i]=0;}
            m_loopLibrary.swap(nextLoops);m_gridLibrary.swap(nextGrids);
        }
        for(int i=0;i<2;++i){m_queue[i]=bundle.lists[i].items;m_queueCurrent[i]=full?dj_session::currentFor(bundle.lists[i],bundle.decks[i].item):dj_session::currentFor(bundle.lists[i],{m_savedPath[i],m_savedSubsong[i]});}
        m_queueInfoAttempted.clear();m_forceQueueInfo=true;
        if(full){
            m_crossfade=(float)bundle.crossfade;m_fadeSeconds=bundle.fade;m_linearFade=bundle.linear;
            m_restoring=true;try{
                for(int i=0;i<2;++i){auto saved=bundle.decks[i];m_speed[i]=saved.speed;m_speedRange[i]=saved.range;m_transpose[i]=saved.transpose;m_fixedPitch[i]=saved.fixed;m_volume[i]=(float)saved.volume;m_loopSnap[i]=saved.snap;if(i)cfg_loopSnapB=saved.snap?1:0;else cfg_loopSnapA=saved.snap?1:0;
                    if(saved.item.path.empty())continue;

                    m_importDeck[i]=saved;m_applyImport[i]=true;m_savedPath[i]=saved.item.path;m_savedSubsong[i]=saved.item.subsong;
                    LoadDeck(i,saved.item.path.c_str(),saved.item.subsong);SeekDeck(i,saved.position);
                }
            }catch(...){m_restoring=false;throw;}m_restoring=false;
        }
        UpdateQueue(0);UpdateQueue(1);SaveSession();Invalidate();
    }
    static bool BundleFileDialog(HWND owner,bool save,std::wstring& path,bool english){
        std::vector<wchar_t> name(32768,0);if(save)wcscpy_s(name.data(),name.size(),L"DJ Decks.dj-decks.json");
        OPENFILENAMEW dialog={sizeof(dialog)};dialog.hwndOwner=owner;dialog.lpstrFile=name.data();dialog.nMaxFile=(DWORD)name.size();dialog.lpstrFilter=L"DJ Decks (*.dj-decks.json;*.json)\0*.dj-decks.json;*.json\0All files\0*.*\0\0";dialog.lpstrDefExt=L"json";
        dialog.lpstrTitle=save?(english?L"Save DJ Decks session":L"Enregistrer une session DJ Decks"):(english?L"Load DJ Decks session":L"Charger une session DJ Decks");
        dialog.Flags=OFN_EXPLORER|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|(save?OFN_OVERWRITEPROMPT:OFN_FILEMUSTEXIST);
        bool result=save?GetSaveFileNameW(&dialog)!=FALSE:GetOpenFileNameW(&dialog)!=FALSE;if(result)path=name.data();else if(CommDlgExtendedError()!=0)throw std::runtime_error("Cannot open session file dialog");return result;
    }
    void CDjPanel::DistributeActivePlaylist(){
        bool en=g_english.load();auto owner=m_settings.m_hWnd?m_settings.m_hWnd:m_hWnd;
        try{
            if(m_pendingLoad[0]||m_pendingLoad[1])throw std::runtime_error("Wait until deck loading completes / Attendez la fin du chargement");
            auto manager=playlist_manager::get();auto active=manager->get_active_playlist();
            if(active==pfc_infinite)throw std::runtime_error("No active foobar playlist / Aucune playlist foobar active");
            metadb_handle_list handles;manager->playlist_get_all_items(active,handles);
            if(handles.get_count()==0)throw std::runtime_error("The active playlist is empty / La playlist active est vide");
            if(handles.get_count()>20000)throw std::runtime_error("At most 20000 tracks / Maximum 20000 pistes");
            std::vector<dj_queue::Item> source;source.reserve(handles.get_count());
            for(t_size n=0;n<handles.get_count();++n)source.push_back({handles[n]->get_path(),handles[n]->get_subsong_index()});
            dj_session::Bundle bundle;
            if(!dj_queue::alternate(source,bundle.lists[0].items,bundle.lists[1].items))throw std::runtime_error("Invalid playlist entries / Entrées de playlist invalides");
            bundle=dj_session::decode(dj_session::encode(bundle)); // Validate storage limits before confirmation/mutation.
            pfc::string8 name;manager->playlist_get_name(active,name);
            std::wstring message=(en?L"Distribute active playlist: ":L"Répartir la playlist active : ");message+=pfc::stringcvt::string_wide_from_utf8(name).get_ptr();
            message+=L"\n\nA : "+std::to_wstring(bundle.lists[0].items.size())+L"   /   B : "+std::to_wstring(bundle.lists[1].items.size());
            message+=en?L"\n\nReplace both deck playlists? Tracks 1, 3, 5… go to A; 2, 4, 6… to B. Loaded deck playback is retained. AUTO and previews stop. A recovery snapshot will be saved. The foobar playlist is unchanged.":L"\n\nRemplacer les deux playlists des platines ? Pistes 1, 3, 5… sur A ; 2, 4, 6… sur B. La lecture chargée est conservée. AUTO et préécoute s’arrêtent. Une sauvegarde de récupération sera créée. La playlist foobar reste inchangée.";
            if(::MessageBoxW(owner,message.c_str(),L"DJ Decks",MB_OKCANCEL|MB_ICONQUESTION)!=IDOK)return;
            if(!dj_session::write(SessionFile("foo_dj-before-import.json"),dj_session::serialize(CaptureBundle(true))))throw std::runtime_error("Cannot save recovery snapshot; distribution cancelled / Sauvegarde de sécurité impossible ; répartition annulée");
            ApplyBundle(bundle,false);
        }catch(const std::exception& e){auto message=pfc::stringcvt::string_wide_from_utf8(e.what());::MessageBoxW(owner,message.get_ptr(),L"DJ Decks",MB_OK|MB_ICONERROR);}
    }
    void CDjPanel::ExportBundle(bool full){
        bool en=g_english.load();auto owner=::GetAncestor(m_hWnd,GA_ROOT);try{std::wstring path;if(!BundleFileDialog(owner,true,path,en))return;auto text=dj_session::serialize(CaptureBundle(full));if(!dj_session::write(path,text))throw std::runtime_error("Cannot save file; previous file retained / Enregistrement impossible ; ancien fichier conservé");}
        catch(const std::exception& e){auto text=pfc::stringcvt::string_wide_from_utf8(e.what());::MessageBoxW(owner,text.get_ptr(),L"DJ Decks",MB_OK|MB_ICONERROR);}
    }
    void CDjPanel::ImportBundle(bool recovery){
        bool en=g_english.load();auto owner=::GetAncestor(m_hWnd,GA_ROOT);try{
            std::wstring path;if(recovery)path=SessionFile("foo_dj-before-import.json");else if(!BundleFileDialog(owner,false,path,en))return;
            const auto bundle=dj_session::parse(dj_session::read(path));if(m_pendingLoad[0]||m_pendingLoad[1])throw std::runtime_error("Wait until deck loading completes / Attendez la fin du chargement");
            bool restore=false;
            if(bundle.full){auto answer=::MessageBoxW(owner,en?L"Both playlists will be replaced.\n\nAlso restore deck tracks, positions and mixer settings?\nYes: complete session, paused.\nNo: playlists only, deck playback retained.\nCancel: keep everything.":L"Les deux playlists seront remplacées.\n\nRestaurer aussi les morceaux, positions et réglages des platines ?\nOui : session complète, en pause.\nNon : playlists seules, lecture des platines conservée.\nAnnuler : ne rien changer.",L"DJ Decks",MB_YESNOCANCEL|MB_ICONQUESTION);if(answer==IDCANCEL)return;restore=answer==IDYES;}
            else if(::MessageBoxW(owner,en?L"Replace both playlists? Deck playback will be retained.":L"Remplacer les deux playlists ? La lecture des platines sera conservée.",L"DJ Decks",MB_OKCANCEL|MB_ICONQUESTION)!=IDOK)return;
            auto previous=dj_session::serialize(CaptureBundle(true));if(!dj_session::write(SessionFile("foo_dj-before-import.json"),previous))throw std::runtime_error("Cannot save recovery snapshot; import cancelled / Sauvegarde de sécurité impossible ; chargement annulé");
            ApplyBundle(bundle,restore);
            bool failed=false;if(restore)for(int i=0;i<2;++i)if(!bundle.decks[i].item.path.empty()&&!m_pendingLoad[i]){std::lock_guard<std::mutex> lock(m_lock);failed=failed||!m_deck[i].loaded;}
            if(failed)::MessageBoxW(owner,en?L"Session loaded paused. Some files could not be opened; references are retained. Reconnect the source drive or check the paths, then reload the session.":L"Session chargée en pause. Certains fichiers sont indisponibles ; leurs références sont conservées. Reconnectez le disque ou vérifiez les chemins, puis rechargez la session.",L"DJ Decks",MB_OK|MB_ICONINFORMATION);
        }catch(const std::exception& e){auto text=pfc::stringcvt::string_wide_from_utf8(e.what());::MessageBoxW(owner,text.get_ptr(),L"DJ Decks",MB_OK|MB_ICONERROR);}
    }

	LRESULT CDjPanel::OnRestoreSession(UINT, WPARAM, LPARAM, BOOL &) {
		try {
			picojson::value loops;
            if(picojson::parse(loops,ReadSessionFile("foo_dj-loops.json")).empty()&&loops.is<picojson::object>())m_loopLibrary=loops.get<picojson::object>();
            picojson::value grids;
			if (picojson::parse(grids,ReadSessionFile("foo_dj-grids.json")).empty() && grids.is<picojson::object>()) m_gridLibrary = grids.get<picojson::object>();
		} catch (...) {}
		std::string saved;
		try { saved = ReadSessionFile(); } catch (...) {}
		picojson::value root;
		if (saved.empty() || !picojson::parse(root, saved).empty() || !root.is<picojson::array>()) {
			const pfc::string8 legacy = cfg_session.get();
			if (!picojson::parse(root, std::string(legacy.get_ptr())).empty() || !root.is<picojson::array>()) return 0;
		}
		const auto & items = root.get<picojson::array>();
		if (items.size() != 2) return 0;
		m_restoring = true;
		for (int i = 0; i < 2; ++i) {
			const auto & item = items[i];
			if (!item.is<picojson::object>()) continue;
			const auto& chain=item.get("autoChain");if(chain.is<bool>())m_chain[i]=chain.get<bool>();
            const auto& fade=item.get("fadeSeconds");if(fade.is<double>()&&std::isfinite(fade.get<double>()))m_fadeSeconds=(std::max)(0.0,(std::min)(30.0,fade.get<double>()));
            const auto& linear=item.get("linearFade");if(linear.is<bool>())m_linearFade=linear.get<bool>();
            m_queue[i]=dj_queue::decode(item.get("queue"));
            const auto& current=item.get("queueCurrent");
            if(current.is<double>() && current.get<double>()>=0 && current.get<double>()<m_queue[i].size() && std::floor(current.get<double>())==current.get<double>())m_queueCurrent[i]=(int)current.get<double>();
            const auto & path = item.get("path"), &subsong = item.get("subsong"), &position = item.get("position");
			if (!path.is<std::string>() || !subsong.is<double>()) continue;
			const std::string file = path.get<std::string>(); const double sub = subsong.get<double>();
			if (file.empty() || file.size() > 32768 || file.find('\0') != std::string::npos || !std::isfinite(sub) || sub < 0 || sub > 4294967295.0 || std::floor(sub) != sub) continue;
			// Keep the reference even if a removable drive is temporarily unavailable.
			m_savedPath[i] = file; m_savedSubsong[i] = (t_uint32)sub;
			const auto & pitch = item.get("transpose");
            if(pitch.is<double>() && std::isfinite(pitch.get<double>())) m_transpose[i]=(int)(std::max)(-12.0,(std::min)(12.0,std::round(pitch.get<double>())));
            const auto & fixed = item.get("fixedPitch");
			if (fixed.is<bool>()) m_fixedPitch[i] = fixed.get<bool>();
			const auto & range = item.get("speedRange");
            if(range.is<double>() && (range.get<double>()==8 || range.get<double>()==16 || range.get<double>()==50)) m_speedRange[i]=(int)range.get<double>();
            const auto & speed = item.get("speed");
			if (speed.is<double>() && std::isfinite(speed.get<double>())) m_speed[i] = dj_tempo::clamp(speed.get<double>(),m_speedRange[i].load());
            if(!item.get("queue").is<picojson::array>()){m_queue[i].push_back({file,(unsigned)sub});m_queueCurrent[i]=0;}
            LoadDeck(i, file.c_str(), (t_uint32)sub);
			if (position.is<double>() && std::isfinite(position.get<double>()) && position.get<double>() > 0) SeekDeck(i, position.get<double>());
		}
		m_restoring = false;
		UpdateQueue(0);UpdateQueue(1);
		Invalidate();
		return 0;
	}

	// ---------------------------------------------------------------- glisser-deposer

	// Cible de glisser-deposer : recoit ce qui est glisse sur le panneau (depuis la liste d'albums,
	// une playlist ou la bibliotheque de foobar2000, ou depuis l'Explorateur Windows) et le transmet au panneau.
	class CDjDropTarget : public IDropTarget {
	public:
		explicit CDjDropTarget(CDjPanel * panel) : m_panel(panel) {}
		void Detach() { m_panel = nullptr; }

		STDMETHODIMP QueryInterface(REFIID riid, void ** ppv) override {
			if (!ppv) return E_POINTER;
			if (riid == __uuidof(IUnknown) || riid == __uuidof(IDropTarget)) {
				*ppv = static_cast<IDropTarget *>(this);
				AddRef();
				return S_OK;
			}
			*ppv = nullptr;
			return E_NOINTERFACE;
		}
		STDMETHODIMP_(ULONG) AddRef() override { return (ULONG)InterlockedIncrement(&m_refs); }
		STDMETHODIMP_(ULONG) Release() override {
			const LONG n = InterlockedDecrement(&m_refs);
			if (n == 0) delete this;
			return (ULONG)n;
		}

		STDMETHODIMP DragEnter(IDataObject * obj, DWORD, POINTL pt, DWORD * effect) override {
			if (!m_panel) { if (effect) *effect = DROPEFFECT_NONE; return S_OK; }
			return m_panel->DropEnter(obj, pt, effect);
		}
		STDMETHODIMP DragOver(DWORD, POINTL pt, DWORD * effect) override {
			if (!m_panel) { if (effect) *effect = DROPEFFECT_NONE; return S_OK; }
			return m_panel->DropOver(pt, effect);
		}
		STDMETHODIMP DragLeave() override {
			return m_panel ? m_panel->DropLeave() : S_OK;
		}
		STDMETHODIMP Drop(IDataObject * obj, DWORD, POINTL pt, DWORD * effect) override {
			if (!m_panel) { if (effect) *effect = DROPEFFECT_NONE; return S_OK; }
			return m_panel->DropDrop(obj, pt, effect);
		}

	private:
		~CDjDropTarget() {}   // on ne le detruit que via Release()
		LONG m_refs = 1;
		CDjPanel * m_panel;
	};

	// Enregistre la fenetre comme cible de depot.
	void CDjPanel::RegisterDrop() {
		m_dropTarget = new CDjDropTarget(this);
		const HRESULT hr = ::RegisterDragDrop(m_hWnd, m_dropTarget);
		if (FAILED(hr)) {
			FB2K_console_formatter() << "foo_dj : glisser-deposer indisponible (code " << (unsigned)hr << ")";
			m_dropTarget->Release();
			m_dropTarget = nullptr;
		}
	}

	void CDjPanel::RevokeDrop() {
		if (m_dropTarget) {
			::RevokeDragDrop(m_hWnd);
			m_dropTarget->Detach();
			m_dropTarget->Release();
			m_dropTarget = nullptr;
		}
	}

	// Quel deck se trouve sous ce point de l'ecran ? (-1 = aucun)
	int CDjPanel::DeckAtScreenPoint(POINTL pt) {
		POINT p = { pt.x, pt.y };
		ScreenToClient(&p);
		CRect rc;
		GetClientRect(&rc);
		for (int i = 0; i < 2; ++i) {
			if (MakeLayout(i, rc).deck.PtInRect(p)) return i;
		}
		return -1;
	}

	// WebView2 exposes its text/plain drag payload as bounded UTF-16 text through OLE.
	static bool ExtractWebViewItem(IDataObject * obj, pfc::string8 & path, t_uint32 & subsong) {
		FORMATETC fmt = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM medium = {};
		if (FAILED(obj->GetData(&fmt, &medium))) return false;
		struct Release { STGMEDIUM & value; ~Release() { ReleaseStgMedium(&value); } } release{ medium };
		if (medium.tymed != TYMED_HGLOBAL || !medium.hGlobal) return false;
		const SIZE_T bytes = GlobalSize(medium.hGlobal);
		if (bytes < sizeof(wchar_t) || bytes > 16 * 1024 * 1024 || bytes % sizeof(wchar_t)) return false;
		const wchar_t * text = static_cast<const wchar_t *>(GlobalLock(medium.hGlobal));
		if (!text) return false;
		struct Unlock { HGLOBAL value; ~Unlock() { GlobalUnlock(value); } } unlock{ medium.hGlobal };
		const size_t capacity = bytes / sizeof(wchar_t);
		size_t length = 0;
		while (length < capacity && text[length]) ++length;
		if (length == capacity) return false;
		try {
			std::string parsedPath;
			unsigned parsedSubsong = 0;
			pfc::stringcvt::string_utf8_from_wide utf8(text, length);
			if (!dj_drop::parse(utf8.get_ptr(), parsedPath, parsedSubsong)) return false;
			path = parsedPath.c_str();
			subsong = parsedSubsong;
			return true;
		} catch (...) { return false; }
	}

	// Ce qui est glisse contient-il des morceaux utilisables ?
	bool CDjPanel::CanAccept(IDataObject * pObj) {
		pfc::string8 path;
		t_uint32 subsong = 0;
		if (ExtractWebViewItem(pObj, path, subsong)) return true;
		try {
			pfc::com_ptr_t<IDataObject> obj(pObj);
			DWORD effect = DROPEFFECT_COPY;
			bool isNative = false;
			if (ole_interaction::get()->check_dataobject(obj, effect, isNative) == S_OK) return true;
		} catch (...) {}
		// Fichiers glisses depuis l'Explorateur Windows
		FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		return pObj->QueryGetData(&fmt) == S_OK;
	}

	// Extrait le premier morceau de ce qui a ete depose : son adresse foobar2000 et son numero de sous-piste.
	bool CDjPanel::ExtractFirstItem(IDataObject * pObj, pfc::string8 & path, t_uint32 & subsong) {
		if (ExtractWebViewItem(pObj, path, subsong)) return true;
		// 1. Formats internes de foobar2000 (liste d'albums, playlists, bibliotheque...)
		try {
			pfc::com_ptr_t<IDataObject> obj(pObj);
			metadb_handle_list handles;
			const HRESULT hr = ole_interaction::get()->parse_dataobject_immediate(obj, handles);
			if (SUCCEEDED(hr) && handles.get_count() > 0) {
				path = handles[0]->get_path();
				subsong = handles[0]->get_subsong_index();
				return true;
			}
		} catch (...) {}

		// 2. Fichiers glisses depuis l'Explorateur Windows
		FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stg = {};
		if (SUCCEEDED(pObj->GetData(&fmt, &stg))) {
			bool ok = false;
			wchar_t buffer[MAX_PATH * 4] = {};
			if (DragQueryFileW((HDROP)stg.hGlobal, 0, buffer, (UINT)(sizeof(buffer) / sizeof(buffer[0]))) > 0) {
				path = "file://";
				path += pfc::stringcvt::string_utf8_from_wide(buffer).get_ptr();
				subsong = 0;
				ok = true;
			}
			ReleaseStgMedium(&stg);
			if (ok) return true;
		}
		return false;
	}

	HRESULT CDjPanel::DropEnter(IDataObject * obj, POINTL pt, DWORD * effect) {
		m_dropOk = (obj != nullptr) && CanAccept(obj);
		m_dropHover = m_dropOk ? DeckAtScreenPoint(pt) : -1;
		if (effect) *effect = (m_dropHover >= 0) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
		Invalidate();
		return S_OK;
	}

	HRESULT CDjPanel::DropOver(POINTL pt, DWORD * effect) {
		const int hover = m_dropOk ? DeckAtScreenPoint(pt) : -1;
		if (hover != m_dropHover) {
			m_dropHover = hover;
			Invalidate();
		}
		if (effect) *effect = (hover >= 0) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
		return S_OK;
	}

	HRESULT CDjPanel::DropLeave() {
		m_dropOk = false;
		m_dropHover = -1;
		Invalidate();
		return S_OK;
	}

	HRESULT CDjPanel::DropDrop(IDataObject * obj, POINTL pt, DWORD * effect) {
		const int deck = DeckAtScreenPoint(pt);
		m_dropOk = false;
		m_dropHover = -1;
		DWORD result = DROPEFFECT_NONE;
		if (obj && deck >= 0) {
            AddQueue(deck,obj,m_queue[deck].size());
            result=DROPEFFECT_COPY;
		}
		if (effect) *effect = result;
		Invalidate();
		return S_OK;
	}


    std::wstring CDjPanel::QueueLabel(const dj_queue::Item& item) {
        // Cached metadata only: listing a queue must never open a decoder or file.
        try {
            auto h=metadb::get()->handle_create(item.path.c_str(),item.subsong);
            file_info_impl info;
            {
                std::string script=cfg_queueFormat.get().get_ptr();
                if(!m_queueFormat.is_valid()||m_queueFormatText!=script){m_queueFormatText=script;if(!titleformat_compiler::get()->compile(m_queueFormat,script.c_str()))titleformat_compiler::get()->compile_force(m_queueFormat,dj_queue_format::presets[0]);}
                pfc::string8 text;DjQueueDurationHook duration(QueueDuration(item,h));h->format_title(&duration,text,m_queueFormat,nullptr);
                if(!text.is_empty())return dj_queue_format::singleLine(pfc::stringcvt::string_wide_from_utf8(text).get_ptr());
            }
        }catch(...){}
        auto n=item.path.find_last_of("/\\");std::string name=n==std::string::npos?item.path:item.path.substr(n+1);
        std::wstring label=pfc::stringcvt::string_wide_from_utf8(name.c_str()).get_ptr();
        if(item.subsong)label+=L" ["+std::to_wstring(item.subsong)+L"]";return label;
    }
    void CDjPanel::UpdateQueue(int deck,const std::vector<size_t>& selected) {
        EnsureQueueInfo();
        auto& cached=m_queueLabels[deck];cached.clear();
        for(auto& item:m_queue[deck])cached.push_back(QueueLabel(item));
        if(m_queueWindow[deck].m_hWnd) {
            std::vector<std::wstring> labels;size_t n=0;
            for(auto& item:cached){auto label=(cfg_queueNumbers.get()?std::to_wstring(n+1)+L". ":L"")+item;if((int)n==m_queueCurrent[deck])label=L"▶ "+label;labels.push_back(label);++n;}
            m_queueWindow[deck].previewActive=-1;
            if(m_previewOwned&&m_previewDeck==deck)for(size_t n=0;n<m_queue[deck].size();++n)
                if(m_queue[deck][n].path==m_previewItem.path&&m_queue[deck][n].subsong==m_previewItem.subsong){m_queueWindow[deck].previewActive=(int)n;break;}
            m_queueWindow[deck].refresh(labels,selected);
        }Invalidate();
    }
    void CDjPanel::OpenQueue(int deck) {
        auto& window=m_queueWindow[deck];
        window.preview=[this,deck](size_t row){StartPreview(deck,row);};
        window.received=[this,deck](DjQueueWindow* source,const std::vector<size_t>& selection,size_t before,bool moving){if(source!=&m_queueWindow[1-deck])return false;return TransferQueue(1-deck,deck,selection,before,moving);};
        window.accepts=[this](IDataObject* o){return CanAccept(o);};
        window.dropped=[this,deck](IDataObject* o,size_t before){AddQueue(deck,o,before);};
        window.action=[this,deck](int a,std::vector<size_t> s,size_t b){QueueAction(deck,a,s,b);};
        window.open(m_hWnd,deck,g_english.load());
        UpdateQueue(deck,m_queueCurrent[deck]>=0?std::vector<size_t>{(size_t)m_queueCurrent[deck]}:std::vector<size_t>{});
    }
    void CDjPanel::QueueAction(int deck,int action,std::vector<size_t> selected,size_t before) {
        auto& queue=m_queue[deck];
        if(action==5){
            if(queue.empty())return;bool en=g_english.load();auto owner=m_queueWindow[deck].m_hWnd;
            if(::MessageBoxW(owner,en?L"Empty this deck's entire playlist?\nThe loaded track keeps playing. No music files are deleted.":L"Vider toute la liste de cette platine ?\nLe morceau chargé continue à jouer. Aucun fichier musical n’est supprimé.",L"DJ Decks",MB_OKCANCEL|MB_ICONQUESTION)!=IDOK)return;
            CancelAutomation();m_chain[deck]=false;DropPreparation(deck);
            if(m_previewOwned&&m_previewDeck==deck)StopPreview();
            queue.clear();m_queueCurrent[deck]=-1;SaveSession();UpdateQueue(deck);return;
        }
        if(selected.empty())return;
        selected.erase(std::remove_if(selected.begin(),selected.end(),[&](size_t n){return n>=queue.size();}),selected.end());if(selected.empty())return;
        std::sort(selected.begin(),selected.end());
        if(action==0) {auto item=queue[selected[0]];m_queueCurrent[deck]=(int)selected[0];LoadDeck(deck,item.path.c_str(),item.subsong);}
        else if(action==1) {
            int old=m_queueCurrent[deck];
            if(old>=0){if(std::find(selected.begin(),selected.end(),(size_t)old)!=selected.end())m_queueCurrent[deck]=-1;
                else m_queueCurrent[deck]-=(int)std::count_if(selected.begin(),selected.end(),[&](size_t n){return n<(size_t)old;});}
            dj_queue::erase(queue,selected);selected.clear();
        } else {
            if(action==2)before=selected.front()?selected.front()-1:0;
            if(action==3)before=(std::min)(queue.size(),selected.back()+2);
            // Track the active occurrence, even when identical paths appear more than once.
            std::vector<dj_queue::Item> indices;for(size_t n=0;n<queue.size();++n)indices.push_back({std::to_string(n),0});
            dj_queue::move(indices,selected,before);
            int old=m_queueCurrent[deck];if(old>=0)for(size_t n=0;n<indices.size();++n)if(indices[n].path==std::to_string(old))m_queueCurrent[deck]=(int)n;
            selected=dj_queue::move(queue,selected,before);
        }
        SaveSession();UpdateQueue(deck,selected);
    }
    void CDjPanel::AddQueue(int deck,IDataObject* object,size_t before) {
        auto items=ExtractItems(object);struct ReleaseDropHandles{metadb_handle_list& handles;~ReleaseDropHandles(){handles.remove_all();}} releaseDrop{m_dropHandles};if(items.empty())return;
        auto& queue=m_queue[deck];before=(std::min)(before,queue.size());
        if(queue.size()+items.size()>10000){MessageBox(g_english.load()?L"Maximum 10000 tracks per deck.":L"Maximum 10000 morceaux par platine.",L"DJ Decks",MB_OK);return;}
        auto candidate=queue;candidate.insert(candidate.begin()+before,items.begin(),items.end());
        if(dj_queue::encode(candidate).serialize().size()+dj_queue::encode(m_queue[1-deck]).serialize().size()>3*1024*1024) {
            MessageBox(g_english.load()?L"Playlist storage limit reached (3 MiB).":L"Limite de sauvegarde des listes atteinte (3 Mio).",L"DJ Decks",MB_OK);return;
        }
        if(m_queueCurrent[deck]>=(int)before)m_queueCurrent[deck]+=(int)items.size();
        queue.insert(queue.begin()+before,items.begin(),items.end());
        if(m_savedPath[deck].empty()){m_queueCurrent[deck]=(int)before;auto item=queue[before];LoadDeck(deck,item.path.c_str(),item.subsong);}
        SaveSession();UpdateQueue(deck);
    }
    std::vector<dj_queue::Item> CDjPanel::ExtractItems(IDataObject* object) {
        std::vector<dj_queue::Item> items;
        m_dropHandles.remove_all();if(!object)return items;
        // Prefer native foobar handles (including all subsongs) when available.
        try {pfc::com_ptr_t<IDataObject> obj(object);metadb_handle_list handles;
            if(SUCCEEDED(ole_interaction::get()->parse_dataobject_immediate(obj,handles)) && handles.get_count()) {
                m_dropHandles=handles;
                for(t_size n=0;n<handles.get_count()&&n<10001;++n)items.push_back({handles[n]->get_path(),handles[n]->get_subsong_index()});return items;
            }
        }catch(...){}
        FORMATETC fmt={CF_UNICODETEXT,nullptr,DVASPECT_CONTENT,-1,TYMED_HGLOBAL};STGMEDIUM medium={};
        if(SUCCEEDED(object->GetData(&fmt,&medium))) {
            if(medium.tymed==TYMED_HGLOBAL&&medium.hGlobal) {
                auto bytes=GlobalSize(medium.hGlobal);
                if(bytes>=2&&bytes<=16*1024*1024&&bytes%2==0){auto text=(const wchar_t*)GlobalLock(medium.hGlobal);
                    if(text){size_t len=0;while(len<bytes/2&&text[len])++len;
                        if(len<bytes/2){std::string utf=pfc::stringcvt::string_utf8_from_wide(text,len).get_ptr();
                            std::string path;unsigned sub;
                            if(dj_drop::parse(utf,path,sub)) {auto start=utf.find('\n');picojson::value root;
                                if(picojson::parse(root,utf.substr(start+1)).empty())items=dj_queue::decode(root.get("items"),10001);}
                        }GlobalUnlock(medium.hGlobal);}
                }
            }ReleaseStgMedium(&medium);if(!items.empty())return items;
        }
        fmt.cfFormat=CF_HDROP;medium={};
        if(SUCCEEDED(object->GetData(&fmt,&medium))) {
            if(medium.tymed==TYMED_HGLOBAL&&medium.hGlobal){auto count=DragQueryFileW((HDROP)medium.hGlobal,0xFFFFFFFF,nullptr,0);
                for(UINT n=0;n<count&&n<10001;++n){auto len=DragQueryFileW((HDROP)medium.hGlobal,n,nullptr,0);if(!len||len>32768)continue;
                    std::vector<wchar_t> file(len+1);DragQueryFileW((HDROP)medium.hGlobal,n,file.data(),len+1);
                    if(GetFileAttributesW(file.data())&FILE_ATTRIBUTE_DIRECTORY)continue;
                    items.push_back({std::string("file://")+pfc::stringcvt::string_utf8_from_wide(file.data()).get_ptr(),0});}
            }ReleaseStgMedium(&medium);
        }return items;
    }



    // Stems are prepared outside the audio process. The host only mixes mapped PCM.
    void CDjPanel::CancelStems(){
        if(m_stemAbort)m_stemAbort->set();
        if(m_stemJob)m_stemJob->stop();m_stemJob.reset();m_stemAbort.reset();m_stemOwner=-1;
    }
    void CDjPanel::CheckStems(int deck){
        m_stemReady[deck]=false;
        try {
            auto native=filesystem::g_get_native_path(m_savedPath[deck].c_str());
            m_stemKey[deck]=dj_stems::key(pfc::stringcvt::string_wide_from_utf8(native).get_ptr(),m_savedSubsong[deck]);
            if(m_stemKey[deck].empty())return;
            dj_stems::Reader probe(dj_stems::folder(m_stemKey[deck]),m_stemKey[deck]);m_stemReady[deck]=true;
        }catch(...){}
    }
    void CDjPanel::StartStems(int deck){
        if(m_pendingLoad[deck])return;
        if(m_stemJob){MessageBox(g_english.load()?L"One preparation at a time. Click its progress button to cancel.":L"Une préparation à la fois. Cliquez sur sa progression pour annuler.",L"DJ Decks",MB_OK);return;}
        if(m_savedPath[deck].empty())return;
        bool playable=false;{std::lock_guard<std::mutex> lock(m_lock);playable=m_deck[deck].loaded&&m_deck[deck].helper;}
        if(!playable){MessageBox(g_english.load()?L"Load or explicitly resume the track before preparing stems.":L"Chargez ou reprenez explicitement le morceau avant de préparer ses stems.",L"DJ Decks",MB_OK);return;}
        CheckStems(deck);if(m_stemKey[deck].empty()){MessageBox(g_english.load()?L"Stems require a local audio file.":L"Les stems nécessitent un fichier audio local.",L"DJ Decks",MB_OK);return;}
        try{
            auto root=dj_stems::localRoot()+L"\\stems-engine";
            if(GetFileAttributesW((root+L"\\venv\\Scripts\\python.exe").c_str())==INVALID_FILE_ATTRIBUTES || GetFileAttributesW((root+L"\\stems_worker.py").c_str())==INVALID_FILE_ATTRIBUTES){
                MessageBox(g_english.load()?L"Install the optional stem engine using the installer supplied with DJ Decks (stems/install-stems.ps1), then try again.":L"Installez le moteur de stems facultatif avec l'installateur fourni avec DJ Decks (stems/install-stems.ps1), puis réessayez.",L"DJ Decks",MB_OK|MB_ICONINFORMATION);return;
            }
            auto path=m_savedPath[deck];auto subsong=m_savedSubsong[deck];auto key=m_stemKey[deck];
            m_stemAbort=std::make_shared<abort_callback_impl>();auto abort=m_stemAbort;
            m_stemJob=std::make_unique<dj_stems::Job>();m_stemOwner=deck;
            m_stemJob->start(key,[path,subsong,key,abort](dj_stems::Job& job,const std::wstring& output){
                input_helper input;auto location=make_playable_location(path.c_str(),subsong);input.open(service_ptr_t<file>(),location,input_flag_no_looping,*abort);
                file_info_impl info;input.get_info(subsong,info,*abort);
                if(info.get_length()>1800)throw std::runtime_error("Stems: maximum 30 minutes");
                audio_chunk_impl chunk;std::unique_ptr<dj_stems::WaveWriter> writer;uint64_t frames=0;unsigned rate=0,channels=0;
                while(input.run(chunk,*abort)){
                    if(job.cancel)throw std::runtime_error("Cancelled");
                    if(!writer){rate=chunk.get_sample_rate();channels=chunk.get_channel_count();writer=std::make_unique<dj_stems::WaveWriter>(output,rate);}
                    if(chunk.get_sample_rate()!=rate||chunk.get_channel_count()!=channels)throw std::runtime_error("Changing audio format is unsupported for stems");
                    writer->append(chunk.get_data(),chunk.get_sample_count(),channels);frames+=chunk.get_sample_count();
                    if(info.get_length()>0)job.progress=(int)((std::min)(19.0,19.0*frames/rate/info.get_length()));
                }
                if(!writer)throw std::runtime_error("No decoded audio");writer->finish();
                auto native=filesystem::g_get_native_path(path.c_str());
                if(dj_stems::key(pfc::stringcvt::string_wide_from_utf8(native).get_ptr(),subsong)!=key)throw std::runtime_error("Source changed during export. Retry preparation.");
            });
        }catch(const std::exception& e){CancelStems();popup_message::g_show(e.what(),"DJ Decks - stems");}
        Invalidate();
    }
    void CDjPanel::StemsTick(){
        if(!m_stemJob)return;int state=m_stemJob->state.load();if(state<3)return;
        int owner=m_stemOwner;auto error=m_stemJob->error;bool failed=state==4;
        CancelStems();if(owner>=0)CheckStems(owner);Invalidate();
        if(failed)MessageBox(error.c_str(),L"DJ Decks - stems",MB_OK|MB_ICONINFORMATION);
    }
    void CDjPanel::UseStems(int deck,unsigned mask){
        if(m_pendingLoad[deck])return;
        try{
            std::shared_ptr<dj_stems::Reader> reader;
            {std::lock_guard<std::mutex> lock(m_lock);reader=m_deck[deck].stems;}
            if(!reader){CheckStems(deck);if(!m_stemReady[deck])throw std::runtime_error("Stem cache unavailable or source changed. Prepare this track again.");reader=std::make_shared<dj_stems::Reader>(dj_stems::folder(m_stemKey[deck]),m_stemKey[deck]);}
            std::lock_guard<std::mutex> lock(m_lock);auto& d=m_deck[deck];
            if(!d.loaded||!d.helper)return; // Never reopen a file released for tag writing.
            reader->mask=mask;reader->solo=m_stemSolo[deck];for(int k=0;k<4;++k)reader->volume[k]=m_stemVolume[deck][k];
            if(!d.stems){reader->seek(m_outRate?d.framesPlayed/m_outRate:0);d.stems=reader;d.rate=reader->rate;d.channels=2;d.src.clear();d.srcPos=0;d.pending.clear();d.pendingPos=0;d.stretch.reset();d.stretchFlushed=false;d.eof=false;}
            m_stemActive[deck]=true;m_stemMask[deck]=mask;
        }catch(const std::exception& e){popup_message::g_show(e.what(),"DJ Decks - stems");CheckStems(deck);}
        Invalidate();
    }
    void CDjPanel::OriginalAudio(int deck){
        try{std::lock_guard<std::mutex> lock(m_lock);auto& d=m_deck[deck];if(!d.stems)return;
            if(!d.helper||!d.helper->can_seek())return;
            abort_callback_dummy abort;d.helper->seek(m_outRate?d.framesPlayed/m_outRate:0,abort);
            d.stems.reset();d.rate=d.sourceRate;d.channels=d.sourceChannels;d.src.clear();d.srcPos=0;d.pending.clear();d.pendingPos=0;d.stretch.reset();d.stretchFlushed=false;d.eof=false;
            m_stemActive[deck]=false;m_stemMask[deck]=15;
        }catch(const std::exception& e){popup_message::g_show(e.what(),"DJ Decks - stems");}Invalidate();
    }

    void CDjPanel::SetStemVolume(int deck,int stem,float value){
        m_stemVolume[deck][stem]=(std::max)(0.f,(std::min)(1.f,value));
        {std::lock_guard<std::mutex> lock(m_lock);if(m_deck[deck].stems)m_deck[deck].stems->volume[stem]=m_stemVolume[deck][stem];}
        Invalidate();
    }
    void CDjPanel::SetStemVolumeFromX(int deck,int stem,int x,const CRect& client){
        auto r=MakeLayout(deck,client).stemVolume[stem];SetStemVolume(deck,stem,(float)(x-r.left)/(std::max)(1,r.Width()-1));
    }
    void CDjPanel::SoloStem(int deck,int stem){
        if(!m_stemReady[deck]){StartStems(deck);return;}
        auto previous=m_stemSolo[deck];m_stemSolo[deck]^=1<<stem;UseStems(deck,m_stemMask[deck]);if(!m_stemActive[deck])m_stemSolo[deck]=previous;
    }

    void CDjPanel::ToggleStem(int deck,int stem){
        if(!m_stemReady[deck]){StartStems(deck);return;}
        UseStems(deck,(m_stemActive[deck]?m_stemMask[deck]:15)^(1<<stem));
    }
    void CDjPanel::StemsMenu(int deck,CPoint point){
        if(m_stemJob && m_stemOwner==deck){CancelStems();Invalidate();return;}
        if(!m_stemReady[deck]){StartStems(deck);return;}
        HMENU menu=CreatePopupMenu();bool en=g_english.load();
        AppendMenuW(menu,MF_STRING|(m_stemActive[deck]&&m_stemMask[deck]==15?MF_CHECKED:0),1,en?L"Enable all four stems":L"Activer les quatre stems");
        AppendMenuW(menu,MF_STRING|(!m_stemActive[deck]?MF_CHECKED:0),2,en?L"Original audio":L"Son original");
        ClientToScreen(&point);auto choice=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY,point.x,point.y,0,m_hWnd,nullptr);DestroyMenu(menu);
        if(choice==1){m_stemSolo[deck]=0;UseStems(deck,15);}if(choice==2)OriginalAudio(deck);
        m_pressedButton.SetRectEmpty();::ReleaseCapture();Invalidate();
    }

    // Automatic playback control stays on the UI thread. Decoding/mixing remains on the audio thread.
    void CDjPanel::CancelAutomation() {
        if(m_autoChanging)return;
        bool changed=m_alternate;m_alternate=false;
        {std::lock_guard<std::mutex> lock(m_lock);m_autoFade={};}
        if(changed&&!m_restoring)SaveSession();
    }
    bool CDjPanel::NextQueue(int deck,bool play) {
        int next=m_queueCurrent[deck]+1;
        if(next<0||next>=(int)m_queue[deck].size())return false;
        auto item=m_queue[deck][next];m_queueCurrent[deck]=next;
        m_autoChanging=true;LoadDeck(deck,item.path.c_str(),item.subsong);m_autoChanging=false;
        bool ok=false;
        {std::lock_guard<std::mutex> lock(m_lock);auto& d=m_deck[deck];
         ok=d.loaded&&d.helper&&m_error[deck].empty();if(ok)d.playing=play;}
        UpdateQueue(deck);SaveSession();if(ok&&play)EnsureEngine();return ok;
    }
    void CDjPanel::AutomationMenu(int deck,CPoint point) {
        HMENU menu=CreatePopupMenu(),duration=CreatePopupMenu(),curve=CreatePopupMenu();bool en=g_english.load();
        AppendMenuW(menu,MF_STRING|(!m_chain[deck]&&!m_alternate?MF_CHECKED:0),1,en?L"Manual":L"Manuel");
        AppendMenuW(menu,MF_STRING|(m_chain[deck]?MF_CHECKED:0),2,en?L"Auto-next on this deck":L"Enchaîner cette liste");
        AppendMenuW(menu,MF_STRING|(m_alternate?MF_CHECKED:0),3,en?L"Alternate A / B from this deck":L"Alterner A / B depuis cette platine");
        const int seconds[]={0,1,2,3,5,8,10,15,20,30};
        for(int n=0;n<10;++n){auto label=std::to_wstring(seconds[n])+L" s";AppendMenuW(duration,MF_STRING|(m_fadeSeconds==seconds[n]?MF_CHECKED:0),100+n,label.c_str());}
        AppendMenuW(menu,MF_POPUP,(UINT_PTR)duration,en?L"A/B crossfade duration":L"Durée du fondu A/B");
        AppendMenuW(curve,MF_STRING|(!m_linearFade.load()?MF_CHECKED:0),200,en?L"Equal power":L"Puissance constante");
        AppendMenuW(curve,MF_STRING|(m_linearFade.load()?MF_CHECKED:0),201,en?L"Linear":L"Linéaire");
        AppendMenuW(menu,MF_POPUP,(UINT_PTR)curve,en?L"Crossfader curve":L"Courbe du crossfader");
        ClientToScreen(&point);auto choice=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY,point.x,point.y,0,m_hWnd,nullptr);DestroyMenu(menu);
        if(choice==1){CancelAutomation();m_chain[deck]=false;}
        if(choice==2){CancelAutomation();m_prepareSuspended[deck]=false;m_chain[deck]=true;}
        if(choice==3){
            CancelAutomation();m_prepareSuspended[0]=m_prepareSuspended[1]=false;m_chain[0]=m_chain[1]=false;m_autoActive=deck;
            {std::lock_guard<std::mutex> lock(m_lock);
             for(int i=0;i<2;++i)m_altAvailable[i]=m_deck[i].loaded&&m_deck[i].helper&&m_error[i].empty();
             m_deck[1-deck].playing=false;m_crossfade=(float)deck;}
            // Released files remain released; opt-in must not reopen a file being tagged.
            if(m_altAvailable[0]&&m_altAvailable[1])m_alternate=true;
            else MessageBox(en?L"Load a playable track on each deck first. Unloaded files must be resumed explicitly.":L"Chargez un morceau lisible sur chaque platine. Reprenez explicitement les fichiers libérés.",L"DJ Decks",MB_OK|MB_ICONINFORMATION);
        }
        if(choice>=100&&choice<110)m_fadeSeconds=seconds[choice-100];
        if(choice==200||choice==201)m_linearFade=choice==201;
        m_pressedButton.SetRectEmpty();::ReleaseCapture();SaveSession();Invalidate();
    }
    void CDjPanel::AutomationTick() {
        if(m_restoring||m_autoChanging)return;
        struct State{bool playing=false,ended=false,usable=false;double remaining=-1;};State state[2];dj_auto::Fade fade;
        {std::unique_lock<std::mutex> lock(m_lock,std::try_to_lock);if(!lock.owns_lock())return;
         fade=m_autoFade;
         if(m_alternate&&(m_deck[0].loop.enabled||m_deck[1].loop.enabled))return;
         for(int i=0;i<2;++i){auto& d=m_deck[i];state[i].playing=d.playing;
             state[i].usable=d.loaded&&d.helper&&m_error[i].empty();
             state[i].ended=!d.loop.enabled&&state[i].usable&&d.eof&&!d.playing&&d.pendingFrames()==0;
             state[i].remaining=dj_auto::remaining(d.trimEnd>0?d.trimEnd:d.length,m_outRate?d.framesPlayed/m_outRate:0,m_speed[i].load());
         }}
        for(int i=0;i<2;++i)if(state[i].playing)m_autoArmed[i]=true;
        if(!m_alternate) {
            for(int i=0;i<2;++i)if(m_chain[i]&&m_autoArmed[i]&&state[i].ended&&NextPrepared(i)){
                m_autoArmed[i]=false;
                if(!NextQueue(i,true)){
                    bool error=false;{std::lock_guard<std::mutex> lock(m_lock);error=!m_error[i].empty();}
                    if(error){m_chain[i]=false;SaveSession();}
                }
            }
            return;
        }
        if(fade.finished){
            if(!NextPrepared(fade.from))return;
            {std::lock_guard<std::mutex> lock(m_lock);m_autoFade={};}
            m_autoActive=fade.to;m_autoArmed[fade.from]=false;
            m_altAvailable[fade.from]=NextQueue(fade.from,false);return;
        }
        if(fade.active){
            if(!state[fade.to].usable||state[fade.to].ended){
                m_crossfade=(float)fade.from;CancelAutomation();
            }
            return;
        }
        int from=m_autoActive,to=1-from;
        if(!state[from].usable){CancelAutomation();return;}
        if(!m_altAvailable[to]) {
            if(state[to].usable && m_queueCurrent[to]+1<(int)m_queue[to].size()) {
                if(NextPrepared(to))m_altAvailable[to]=NextQueue(to,false);return;
            }
            // If one playlist is exhausted, finish the remaining playlist without replaying tracks.
            if(state[from].ended&&m_autoArmed[from]&&NextPrepared(from)){m_autoArmed[from]=false;if(!NextQueue(from,true))CancelAutomation();}
            return;
        }
        if(!state[to].usable){CancelAutomation();return;}
        bool end=m_autoArmed[from]&&state[from].ended;
        if(!end && !(state[from].playing&&state[from].remaining>=0&&state[from].remaining<=m_fadeSeconds))return;
        double seconds=end?0:(std::min)(m_fadeSeconds,state[from].remaining);
        if(state[to].remaining>=0)seconds=(std::min)(seconds,state[to].remaining);
        {std::lock_guard<std::mutex> lock(m_lock);
         auto& incoming=m_deck[to];
         if(incoming.eof&&incoming.pendingFrames()==0){m_altAvailable[to]=false;return;}
         incoming.playing=true;m_autoArmed[to]=true;
         m_autoFade.begin(from,m_crossfade.load(),seconds,m_outRate);
         if(!m_autoFade.active){m_crossfade=(float)to;m_deck[from].playing=false;}
        }
        EnsureEngine();Invalidate();
    }

	void CDjPanel::TogglePlay(int deck) {
        if(m_pendingLoad[deck]){m_pendingPlay[deck]=!m_pendingPlay[deck];return;}
        if(m_alternate&&!m_autoChanging) {
            {std::lock_guard<std::mutex> lock(m_lock);
             if(m_deck[0].playing||m_deck[1].playing){m_deck[0].playing=m_deck[1].playing=false;return;}
             if(m_autoFade.active){m_deck[0].playing=m_deck[1].playing=true;EnsureEngine();return;}
             m_autoActive=deck;m_crossfade=(float)deck;
            }
        }
        if(!ReopenAfterTags(deck)) return;
		std::lock_guard<std::mutex> lock(m_lock);
		Deck & d = m_deck[deck];
		if (!d.loaded) return;

		if (d.playing) {          // Pause
			d.playing = false;
			return;
		}

		// Fichier termine : on revient au debut.
		if (d.eof && d.pendingFrames() == 0) {
			try {
				abort_callback_dummy abort;
				if (!d.stems && !d.helper->can_seek()) {
					m_error[deck] = L"Relecture impossible";
					return;
				}
				if(d.stems)d.stems->seek(d.trimStart);else d.helper->seek(d.trimStart, abort);
				d.src.clear();
				d.srcPos = 0;
				d.pending.clear();
				d.pendingPos = 0;
				d.framesPlayed = d.trimStart*(m_outRate?m_outRate:48000); d.listening={};
				d.eof = false; d.stretch.reset(); d.stretchFlushed = false;
			} catch (...) {
				m_error[deck] = L"Erreur au retour au d\u00e9but";
				return;
			}
		}

		m_error[deck].clear();
		d.playing = true;
		EnsureEngine();
	}

	// Toggle both playing flags under one lock so the mixer observes them together.
	void CDjPanel::PlayBoth() {
        if(m_pendingLoad[0]||m_pendingLoad[1]){for(int i=0;i<2;++i)TogglePlay(i);return;}
        if(m_alternate){TogglePlay(m_autoActive);return;}
        if(!ReopenAfterTags(0) || !ReopenAfterTags(1)) return;
		std::lock_guard<std::mutex> lock(m_lock);
		if (!m_deck[0].loaded || !m_deck[1].loaded) return;
		const bool next[2] = { !m_deck[0].playing, !m_deck[1].playing };
		for (int i = 0; i < 2; ++i) {
			Deck & d = m_deck[i];
			if (next[i] && d.eof && d.pendingFrames() == 0) {
				try {
					abort_callback_dummy abort;
					if (!d.stems && !d.helper->can_seek()) throw pfc::exception("Relecture impossible");
					if(d.stems)d.stems->seek(d.trimStart);else d.helper->seek(d.trimStart, abort);
					d.src.clear(); d.srcPos = 0;
					d.pending.clear(); d.pendingPos = 0;
					d.framesPlayed = d.trimStart*(m_outRate?m_outRate:48000); d.listening={}; d.eof = false; d.stretch.reset(); d.stretchFlushed = false;
				} catch (const std::exception & e) {
					pfc::stringcvt::string_wide_from_utf8 message(e.what());
					m_error[i] = message.get_ptr(); return;
				}
			}
		}
		m_error[0].clear(); m_error[1].clear();
		m_deck[0].playing = next[0];
		m_deck[1].playing = next[1];
		if (next[0] || next[1]) EnsureEngine();
	}

	bool CDjPanel::SnapLoopPoint(int deck,double position,double lower,double upper,double& target){
        bool indicative=false;auto grid=dj_beats::displayed(m_grid[deck],m_cacheBpm[deck],m_manualGrid[deck],m_manualPhase[deck],indicative);
        if(!grid.valid)return false;
        CRect client;GetClientRect(&client);auto layout=MakeLayout(deck,client);double start=0,span=0;ViewWindow(deck,start,span);
        if(span<=0||layout.wave.Width()<=2)return false;
        auto ticks=dj_beats::ticks(grid,start,span,layout.wave.Width()-2);
        return dj_loop::snap(grid.period,grid.offset,ticks.stride,position,lower,upper,target);
    }

    void CDjPanel::SaveLoopPoints(int deck,const dj_loop::Region& points){
        if(m_savedPath[deck].empty())return;
        try{
            dj_loop::remember(m_loopLibrary,GridKey(m_savedPath[deck],m_savedSubsong[deck]),points);
            auto text=picojson::value(m_loopLibrary).serialize();
            if(text.size()>4*1024*1024||!WriteSessionFile(text,"foo_dj-loops.json"))throw std::runtime_error("A/B point save failed");
        }catch(const std::exception&){m_loopNotice[deck]=g_english.load()?L"A/B points kept in memory; saving failed":L"Points A/B en mémoire ; échec de sauvegarde";}
    }

    void CDjPanel::SetLoopSnap(int deck,bool enabled){
        std::unique_lock<std::mutex> lock(m_lock);auto& d=m_deck[deck];auto points=d.loop;bool changed=false;
        auto fail=[&](const wchar_t* fr,const wchar_t* en){m_loopNotice[deck]=g_english.load()?en:fr;Invalidate();};
        if(enabled&&points.a>=0){
            if(m_pendingLoad[deck]){fail(L"Q : attendez la fin du chargement",L"Q: wait until loading finishes");return;}
            double end=d.trimEnd>0?d.trimEnd:d.length;bool hasB=points.b>=0;
            double a=points.a,b=points.b;
            if(!SnapLoopPoint(deck,points.a,d.trimStart,end-(hasB?0.100000001:0.001),a)){
                fail(L"Q : aucun repère disponible ; points conservés",L"Q: no usable grid line; points retained");return;
            }
            if(hasB&&!SnapLoopPoint(deck,points.b,a+0.100000001,end,b)){
                // At the final visible line, use the preceding line for A if possible.
                double earlier=a;
                if(!SnapLoopPoint(deck,points.a,d.trimStart,a-0.100000001,earlier)||!SnapLoopPoint(deck,points.b,earlier+0.100000001,end,b)){
                    fail(L"Q : impossible de caler les deux points ; points conservés",L"Q: cannot fit both points; points retained");return;
                }a=earlier;
            }
            auto aligned=points;aligned.a=a;aligned.b=hasB?b:-1;
            if(aligned.enabled&&d.helper&&!aligned.contains(d.framesPlayed/(m_outRate?m_outRate:48000))){
                try{if(!d.stems&&!d.helper->can_seek())throw std::runtime_error("Cannot seek");abort_callback_dummy abort;dj_loop::restart(d,a,m_outRate?m_outRate:48000,abort);}
                catch(const std::exception&){fail(L"Q : repositionnement impossible ; points conservés",L"Q: cannot reposition; points retained");return;}
            }
            d.loop=aligned;points=aligned;changed=true;
        }
        m_loopSnap[deck]=enabled;if(deck)cfg_loopSnapB=enabled?1:0;else cfg_loopSnapA=enabled?1:0;
        m_loopNotice[deck].clear();lock.unlock();if(changed)SaveLoopPoints(deck,points);Invalidate();
    }

    void CDjPanel::EditLoop(int deck,int action){
        std::unique_lock<std::mutex> lock(m_lock);auto& d=m_deck[deck];
        auto fail=[&](const wchar_t* fr,const wchar_t* en){m_loopNotice[deck]=g_english.load()?en:fr;Invalidate();};
        if(action==3){d.loop={};auto points=d.loop;m_loopNotice[deck].clear();lock.unlock();SaveLoopPoints(deck,points);Invalidate();return;}
        if(!d.loaded||!d.helper||m_pendingLoad[deck]){fail(L"Chargez un morceau avant de poser la boucle",L"Load a track before setting a loop");return;}
        if(!d.stems&&!d.helper->can_seek()){fail(L"Boucle : positionnement impossible",L"Loop: seeking unavailable");return;}
        double now=d.framesPlayed/(m_outRate?m_outRate:48000);
        if(action<2&&m_loopSnap[deck]){
            double end=d.trimEnd>0?d.trimEnd:d.length;
            double lower=action==0?d.trimStart:(std::max)(d.trimStart,d.loop.a+0.100000001),upper=action==0?end-0.001:end;
            double snapped=now;
            if((action==1&&d.loop.a<0)||!SnapLoopPoint(deck,now,lower,upper,snapped)){fail(L"Q : aucun repère disponible ici. Vérifiez BPM, grille et zoom.",L"Q: no usable grid line here. Check BPM, grid and zoom.");return;}
            now=snapped;
        }
        if(action==0){if(d.loop.enabled){fail(L"Désactivez la boucle pour modifier A",L"Disable the loop before editing A");return;}d.loop.setA(now);}
        if(action==1){if(d.loop.enabled){fail(L"Désactivez la boucle pour modifier B",L"Disable the loop before editing B");return;}if(!d.loop.setB(now)){fail(L"Placez B au moins 100 ms après A",L"Set B at least 100 ms after A");return;}}
        if(action==2){
            if(!d.loop.enabled){
                if(!d.loop.valid()){fail(L"Posez A puis B pour activer la boucle",L"Set A then B before enabling the loop");return;}
                if(m_autoFade.active||m_autoFade.finished){fail(L"Attendez la fin du fondu AUTO",L"Wait until the AUTO fade finishes");return;}
                try{abort_callback_dummy abort;dj_loop::restart(d,d.loop.a,m_outRate?m_outRate:48000,abort);}catch(const std::exception&){fail(L"Impossible de revenir à A",L"Cannot seek to A");return;}
            }
            d.loop.enabled=!d.loop.enabled;
        }
        m_loopNotice[deck].clear();auto points=d.loop;lock.unlock();if(action<2)SaveLoopPoints(deck,points);Invalidate();
    }

    // Saute a l'instant donne (en secondes) du fichier du deck, en lecture ou en pause.
	void CDjPanel::SeekDeck(int deck, double seconds) {
        if(m_pendingLoad[deck]){m_pendingSeek[deck]=seconds;return;}
		std::lock_guard<std::mutex> lock(m_lock);
		Deck & d = m_deck[deck];
		if (!d.loaded) return;
        if(!d.helper) { m_error[deck]=g_english.load()?L"Finish analysis, then press Play":L"Terminez l'analyse puis cliquez Lecture"; return; }

		seconds = (std::max)(d.trimStart, (std::min)(seconds, (std::max)(d.trimStart, (d.trimEnd>0?d.trimEnd:d.length) - 0.001)));
		try {
			abort_callback_dummy abort;
			if (!d.stems && !d.helper->can_seek()) {
				m_error[deck] = L"Positionnement impossible";
				return;
			}
			if(d.stems)d.stems->seek(seconds);else d.helper->seek(seconds, abort);
			d.src.clear();
			d.srcPos = 0;
			d.pending.clear();
			d.pendingPos = 0;
			d.eof = false; d.stretch.reset(); d.stretchFlushed = false;
			d.framesPlayed = (uint64_t)(seconds * (double)(m_outRate?m_outRate:48000));
            if(d.loop.enabled&&!d.loop.contains(seconds))d.loop.enabled=false;
			m_error[deck].clear();
            // A seek changes source position, not the user's AUTO mode. During
            // a transition keep its direction and current gain, retiming only
            // the remainder against the new source positions/useful ends.
            if(m_alternate&&!m_autoChanging&&m_autoFade.active){
                double duration=m_fadeSeconds;
                for(int i=0;i<2;++i){auto& source=m_deck[i];double remaining=dj_auto::remaining(source.trimEnd>0?source.trimEnd:source.length,source.framesPlayed/(m_outRate?m_outRate:48000),m_speed[i].load());if(remaining>=0)duration=(std::min)(duration,remaining);}
                int from=m_autoFade.from;m_autoFade.begin(from,m_crossfade.load(),duration,m_outRate?m_outRate:48000);
                if(!m_autoFade.active){m_crossfade=(float)m_autoFade.to;m_deck[from].playing=false;}
            }
		} catch (const std::exception & e) {
			pfc::stringcvt::string_wide_from_utf8 message(e.what());
			m_error[deck] = std::wstring(L"Positionnement : ") + message.get_ptr();
		}
	}

	// Demarre le thread du moteur s'il ne tourne pas deja.
	void CDjPanel::EnsureEngine() {
		if (m_thread.joinable() && m_engineFinished) {
			m_thread.join();      // le moteur s'etait arrete (erreur) : on le relance
		}
		if (!m_thread.joinable()) {
			m_stop = false;
			m_engineFinished = false;
			m_thread = std::thread([this] { EngineThread(); });
		}
	}

	void CDjPanel::StopEngine() {
		m_stop = true;
		if (m_thread.joinable()) m_thread.join();
	}

    void CDjPanel::StopPreview(bool wait){
        m_previewOverview.cancel(wait);m_previewPlayer.cancel(wait);m_previewActive=false;m_previewOwned=false;m_previewDeck=-1;m_previewWindow.close();
        for(auto& window:m_queueWindow)if(window.list){window.previewActive=-1;::InvalidateRect(window.list,nullptr,FALSE);}
        ConfigureCue();
    }
    void CDjPanel::PreviewTick(){
        if(m_previewOwned&&m_previewWindow.m_hWnd&&GetTickCount64()-m_previewPaint>=50){m_previewPaint=GetTickCount64();m_previewWindow.update(m_previewVisual.snapshot(m_cueOutput.latency()));}
        if(m_previewOwned&&!m_previewPlayer.active()){
            auto error=m_previewPlayer.lastError();StopPreview();
            if(!error.empty()){auto text=pfc::stringcvt::string_wide_from_utf8(error.c_str());::MessageBoxW(m_hWnd,text.get_ptr(),L"DJ Decks - Preview",MB_OK|MB_ICONINFORMATION);}
        }
    }
    void CDjPanel::StartPreview(int deck,size_t row){
        if(row>=m_queue[deck].size())return;auto item=m_queue[deck][row];
        if(m_previewOwned&&m_previewDeck==deck&&item.path==m_previewItem.path&&item.subsong==m_previewItem.subsong){StopPreview();return;}
        if(cfg_cueBackend.get()==GUID{}){CRect client;GetClientRect(&client);CueMenu(CPoint(client.left+20,client.top+40));return;}
        m_previewActive=true;m_previewOwned=true;m_previewDeck=deck;m_previewItem=item;
        ConfigureCue();
        auto token=m_cueOutput.token();m_previewVisual.reset(token);m_previewPaint=0;
        m_previewWindow.stopped=[this]{StopPreview();};
        m_previewWindow.seeked=[this](double seconds){SeekPreview(seconds);};
        m_previewWindow.open(m_hWnd,QueueLabel(item),g_english.load(),dj_skin::accent(deck),m_queueWindow[deck].m_hWnd);
        m_previewPlayer.start({item,(int)cfg_previewMode.get(),token});
        m_previewOverview.start({item,(int)cfg_previewMode.get(),token});
        for(int i=0;i<2;++i)UpdateQueue(i,m_queueWindow[i].list?m_queueWindow[i].selected():std::vector<size_t>{});
    }
    void CDjPanel::SeekPreview(double seconds){
        if(!m_previewOwned||!std::isfinite(seconds)||seconds<0)return;
        // Reconfigure only cue, fencing queued/late blocks from the old excerpt.
        // Retain the overview and window; no deck/list loading or seeking here.
        m_previewOverview.cancel();m_previewPlayer.cancel();ConfigureCue();
        auto token=m_cueOutput.token();m_previewVisual.restart(token,seconds);
        m_previewWindow.update(m_previewVisual.snapshot(0));m_previewPaint=0;
        m_previewPlayer.start({m_previewItem,(int)cfg_previewMode.get(),token,seconds});
        m_previewOverview.start({m_previewItem,(int)cfg_previewMode.get(),token,seconds});
    }

    std::string CDjPanel::PreviewOverviewKey(const dj_queue::Item& item){
        auto native=filesystem::g_get_native_path(item.path.c_str());if(native.is_empty())return {};
        auto wide=pfc::stringcvt::string_wide_from_utf8(native);WIN32_FILE_ATTRIBUTE_DATA info={};
        if(!::GetFileAttributesExW(wide.get_ptr(),GetFileExInfoStandard,&info))return {};
        ULARGE_INTEGER size={},modified={};size.HighPart=info.nFileSizeHigh;size.LowPart=info.nFileSizeLow;
        modified.HighPart=info.ftLastWriteTime.dwHighDateTime;modified.LowPart=info.ftLastWriteTime.dwLowDateTime;
        return item.path+"\n"+std::to_string(item.subsong)+"\n"+std::to_string(size.QuadPart)+"\n"+std::to_string(modified.QuadPart);
    }
    void CDjPanel::ScanPreviewOverview(const PreviewRequest& request,abort_callback_impl& abort){
        // This optional worker never supplies playback audio. Sampling work is
        // bounded, cancelable and lower priority than the preview decoder.
        ::SetThreadPriority(::GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
        auto key=PreviewOverviewKey(request.item);if(key.empty())return;
        auto overview=m_previewOverviewCache.get(key);
        if(overview.length>0){m_previewVisual.overview(request.token,overview.length,overview.peaks,overview.known,overview.complete());if(overview.complete())return;}
        const auto started=GetTickCount64();input_helper input;
        input.open(service_ptr_t<file>(),make_playable_location(request.item.path.c_str(),request.item.subsong),input_flag_no_looping,abort);
        file_info_impl info;input.get_info(request.item.subsong,info,abort);
        if(!std::isfinite(info.get_length())||info.get_length()<=0||!input.can_seek())return;
        if(overview.length!=info.get_length())overview={};overview.length=info.get_length();
        for(auto index:dj_preview::overviewOrder()){
            abort.check();if(overview.known[index])continue;if(GetTickCount64()-started>=2000)break;
            double position=index==0?0:index==31?(std::max)(0.,overview.length-.04):(index+.5)*overview.length/32.;
            input.seek(position,abort);audio_chunk_impl chunk;float peak=0;
            if(input.run(chunk,abort)){
                auto channels=chunk.get_channel_count();if(channels<1||channels>256)return;
                const auto data=chunk.get_data();for(size_t n=0;n<chunk.get_sample_count();n+=8)
                    for(unsigned c=0;c<(std::min)(channels,2u);++c){double value=std::abs((double)data[n*channels+c]);if(std::isfinite(value))peak=(std::max)(peak,(float)(std::min)(1.,value));}
            }
            overview.peaks[index]=peak;overview.known[index]=true;m_previewOverviewCache.put(key,overview);
            m_previewVisual.overview(request.token,overview.length,overview.peaks,overview.known,overview.complete());
            abort.sleep(.001);
        }
    }

    void CDjPanel::PlayPreview(const PreviewRequest& request,abort_callback_impl& abort){
        // A separate decoder and no deck/session references: previews never load,
        // seek or consume either deck and never count towards scrobbling.
        auto native=filesystem::g_get_native_path(request.item.path.c_str());
        if(native.is_empty())throw std::runtime_error("Preview supports local files / fichiers locaux uniquement");
        {
            input_helper input;auto location=make_playable_location(request.item.path.c_str(),request.item.subsong);
            input.open(service_ptr_t<file>(),location,input_flag_no_looping,abort);
            file_info_impl info;input.get_info(request.item.subsong,info,abort);
            auto spans=dj_preview::planAt(info.get_length(),request.mode,request.position);
            m_previewVisual.begin(request.token,spans,info.get_length());

            double elapsedTotal=0;
            for(auto span:spans){
                abort.check();if(span.start>0){if(!input.can_seek())throw std::runtime_error("Cannot seek to the middle / Positionnement au milieu impossible");input.seek(span.start,abort);}
                auto origin=std::chrono::steady_clock::now();double elapsed=0;
                size_t played=0,target=0;unsigned rate=0;
                while(!target||played<target){
                    audio_chunk_impl chunk;if(!input.run(chunk,abort))break;
                    unsigned hz=chunk.get_sample_rate(),channels=chunk.get_channel_count();
                    if(hz<1000||hz>768000||channels<1||channels>256)throw std::runtime_error("Unsupported preview audio format");
                    if(rate&&hz!=rate)throw std::runtime_error("Preview sample rate changed");
                    rate=hz;target=(size_t)std::llround(span.seconds*rate);if(!target)break;
                    auto data=chunk.get_data();size_t total=chunk.get_sample_count();
                    for(size_t offset=0;offset<total&&played<target;){
                        abort.check();size_t count=(std::min)(size_t(512),(std::min)(total-offset,target-played));
                        m_previewVisual.add(request.token,span.start+(double)played/rate,rate,data+offset*channels,count,channels);
                        std::vector<audio_sample> samples(count*2);double volume=m_cueVolume.load();volume*=volume;
                        for(size_t n=0;n<count;++n){double fade=(std::min)(1.,(std::min)((played+n+1)/(rate*.005),(target-played-n)/(rate*.005)));
                            samples[n*2]=(audio_sample)(data[(offset+n)*channels]*volume*fade);
                            samples[n*2+1]=(audio_sample)(data[(offset+n)*channels+(channels>1?1:0)]*volume*fade);}
                        auto due=origin+std::chrono::duration<double>(elapsed);auto now=std::chrono::steady_clock::now();
                        if(due>now)abort.sleep(std::chrono::duration<double>(due-now).count());
                        while(!m_cueOutput.push(samples,rate,request.token)){
                            abort.check();if(m_cueOutput.token()!=request.token)return;
                            auto error=m_cueOutput.lastError();if(!error.empty())throw std::runtime_error(error);abort.sleep(.002);
                        }
                        played+=count;offset+=count;elapsed+=(double)count/rate;
                        m_previewVisual.submitted(request.token,elapsedTotal+elapsed);
                    }
                }
                elapsedTotal+=elapsed;
            }
        } // release the input before completion (including tag-writing handoff)
        abort.sleep(.3); // let the final buffered cue block drain before restoring deck cue
    }

    void CDjPanel::RefreshTrimPreparations(){
        ClearPreparations();for(int i=0;i<2;++i)if(m_pendingLoad[i]){if(TrimOptions().enabled())RequestPreparation(i,m_pendingItem[i]);else{auto item=m_pendingItem[i];bool play=m_pendingPlay[i];double seek=m_pendingSeek[i];m_pendingLoad[i]=m_pendingPlay[i]=false;m_pendingSeek[i]=-1;CommitLoadDeck(i,item.path.c_str(),item.subsong);if(seek>=0)SeekDeck(i,seek);if(play)TogglePlay(i);}}
    }
    void CDjPanel::SettingsCommand(int choice){
        bool en=g_english.load();
            if(choice==221){m_pressedButton.SetRectEmpty();::ReleaseCapture();auto detected=dj_scrobble::probe();::MessageBoxW(m_hWnd,detected.message(en).c_str(),L"DJ Decks - foo_scrobble",MB_OK|MB_ICONINFORMATION);}
            if(choice==306){m_queueInfoAttempted.clear();m_forceQueueInfo=true;for(int i=0;i<2;++i)UpdateQueue(i,m_queueWindow[i].list?m_queueWindow[i].selected():std::vector<size_t>{});}
            if(choice>=300&&choice<=305){m_pressedButton.SetRectEmpty();::ReleaseCapture();QueueFormatMenu((int)choice);}

            if(choice>=210&&choice<=212){g_replaygain=(int)choice-210;cfg_replaygain=(int)choice-210;}
            if(choice==220){
                bool enable=!g_scrobble.load();
                if(enable&&!dj_scrobble::service().is_valid()){m_pressedButton.SetRectEmpty();::ReleaseCapture();auto detected=dj_scrobble::probe();::MessageBoxW(m_hWnd,detected.message(en).c_str(),L"DJ Decks - foo_scrobble",MB_OK|MB_ICONINFORMATION);}
                else{g_scrobble=enable;cfg_scrobble=enable;g_scrobbleReady=false;{std::lock_guard<std::mutex> lock(m_lock);for(auto& d:m_deck)d.listening.discardUnsubmitted();}m_lastScrobbleTick=0;ScrobbleTick();}
            }

            if(choice==200){m_pressedButton.SetRectEmpty();::ReleaseCapture();std::set<std::wstring> keys;{std::lock_guard<std::mutex> lock(m_lock);for(int i=0;i<2;++i)if(m_deck[i].stems)keys.insert(dj_stems::wide(m_stemKey[i]));}if(m_stemJob)keys.insert(dj_stems::wide(m_stemJob->sourceKey));dj_stems::manageCache(m_settings.m_hWnd?m_settings.m_hWnd:m_hWnd,en,keys);for(int i=0;i<2;++i)CheckStems(i);}
            if(choice==1 || choice==2){g_english=choice==2;cfg_language=choice==2;}
            if(choice==10||choice==11){dj_skin::flat=choice==11;cfg_flat=choice==11;}
            if(choice>=100&&choice<108){dj_skin::colorIndex[0]=(int)choice-100;cfg_colorA=(int)choice-100;}
            if(choice>=120&&choice<128){dj_skin::colorIndex[1]=(int)choice-120;cfg_colorB=(int)choice-120;}
            for(auto& window:m_queueWindow)if(window.m_hWnd)::RedrawWindow(window.m_hWnd,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);

        m_settings.english=g_english.load();m_settings.accent=DeckAccent(0);
        m_pressedButton.SetRectEmpty();::ReleaseCapture();Invalidate();
    }

    double CDjPanel::MidiCurrent(int action){
        if(action==dj_midi::crossfade)return m_crossfade.load();
        if(action==dj_midi::volumeA||action==dj_midi::volumeB)return m_volume[action-dj_midi::volumeA].load();
        if(action==dj_midi::tempoA||action==dj_midi::tempoB){int deck=action-dj_midi::tempoA;double range=dj_tempo::range(m_speedRange[deck].load());return (m_speed[deck].load()-1+range)/(2*range);}
        return 0;
    }
    void CDjPanel::MidiApply(int action,double value){
        if(action==dj_midi::crossfade){CancelAutomation();m_crossfade=(float)value;}
        else if(action==dj_midi::volumeA||action==dj_midi::volumeB)m_volume[action-dj_midi::volumeA]=(float)value;
        else if(action==dj_midi::tempoA||action==dj_midi::tempoB){int d=action-dj_midi::tempoA;double r=dj_tempo::range(m_speedRange[d].load());m_speed[d]=1-r+2*r*value;}
        else if(action==dj_midi::playA||action==dj_midi::playB)TogglePlay(action-dj_midi::playA);
        else if(action==dj_midi::both)PlayBoth();
        else if(action==dj_midi::startA||action==dj_midi::startB)SeekDeck(action-dj_midi::startA,0);
        else if(action==dj_midi::unloadA||action==dj_midi::unloadB)ReleaseForTags(action-dj_midi::unloadA);
        else if(action==dj_midi::cueA||action==dj_midi::cueB){int d=action-dj_midi::cueA;if(cfg_cueBackend.get()!=GUID{}){StopPreview();m_cue[d]=!m_cue[d].load();ConfigureCue();}}
        else if(action>=dj_midi::loopAA&&action<=dj_midi::loopB)EditLoop((action-dj_midi::loopAA)/3,(action-dj_midi::loopAA)%3);
        else if(action==dj_midi::bpmA||action==dj_midi::bpmB)MatchTempo(action-dj_midi::bpmA);
        else if(action==dj_midi::keyA||action==dj_midi::keyB)MatchKey(action-dj_midi::keyA);
        m_dragDirty=true;
    }
    void CDjPanel::MidiInitialize(){
        m_midiRouter.pickup=cfg_midiPickup.get()!=0;
        if(!dj_midi::load(cfg_midiMappings.get().get_ptr(),m_midiProfiles)){m_midiStatus=L"MIDI: invalid mappings / associations invalides";return;}
        std::string key=cfg_midiDevice.get().get_ptr();for(auto& p:m_midiProfiles)if(p.device==key)m_midiRouter.bindings=p.bindings;
        if(cfg_midiEnabled.get())MidiConfigure();else m_midiStatus=g_english?L"MIDI disabled":L"MIDI désactivé";
    }
    void CDjPanel::MidiRefreshDevices(){m_midiDevices=dj_midi::devices();}
    void CDjPanel::MidiSave(){
        std::string key=cfg_midiDevice.get().get_ptr();if(key.empty())return;
        auto p=std::find_if(m_midiProfiles.begin(),m_midiProfiles.end(),[&](const dj_midi::Profile& p){return p.device==key;});
        if(p==m_midiProfiles.end()){if(m_midiProfiles.size()>=16){m_midiStatus=L"MIDI: 16 profiles maximum";return;}m_midiProfiles.push_back({key});p=m_midiProfiles.end()-1;}
        p->bindings=m_midiRouter.bindings;cfg_midiMappings=dj_midi::save(m_midiProfiles).c_str();
    }
    void CDjPanel::MidiConfigure(){
        MidiRefreshDevices();m_midiInput.close();m_midiRouter.arm(-1);m_midiRouter.bindings={};m_midiRouter.pickup=cfg_midiPickup.get()!=0;
        std::string key=cfg_midiDevice.get().get_ptr();for(auto& p:m_midiProfiles)if(p.device==key)m_midiRouter.bindings=p.bindings;
        if(!cfg_midiEnabled.get()){m_midiStatus=g_english?L"MIDI disabled":L"MIDI désactivé";return;}
        auto device=std::find_if(m_midiDevices.begin(),m_midiDevices.end(),[&](const dj_midi::Device& d){return d.key==key;});
        if(device==m_midiDevices.end()){m_midiStatus=g_english?L"Selected MIDI input unavailable; select one and retry.":L"Entrée MIDI indisponible ; choisir une entrée et réessayer.";return;}
        auto result=m_midiInput.open(device->id);if(result!=MMSYSERR_NOERROR){wchar_t text[MAXERRORLENGTH]={};midiInGetErrorTextW(result,text,MAXERRORLENGTH);m_midiStatus=L"MIDI: "+std::wstring(text);return;}
        m_midiStatus=(g_english?L"MIDI connected: ":L"MIDI connecté : ")+device->name;
    }
    void CDjPanel::MidiTick(){
        if(!m_midiInput.opened())return;bool lost=false;auto events=m_midiInput.drain(lost);
        if(lost){m_midiInput.close();m_midiRouter.arm(-1);m_midiStatus=g_english?L"MIDI overload: input stopped. Retry connection.":L"Surcharge MIDI : entrée arrêtée. Réessayer la connexion.";if(m_settings.m_hWnd&&m_settings.selected==6)m_settings.refresh();return;}
        bool learned=false;for(auto event:events)learned|=m_midiRouter.receive(event,[&](int a){return MidiCurrent(a);},[&](int a,double v){MidiApply(a,v);});
        if(learned){MidiSave();m_midiStatus=g_english?L"Association saved; release the button before using it.":L"Association enregistrée ; relâcher le bouton avant de l’utiliser.";if(m_settings.m_hWnd&&m_settings.selected==6)m_settings.refresh();}
    }
    std::vector<dj_settings::Item> CDjPanel::MidiValues(){
        using namespace dj_settings;bool en=g_english.load();std::vector<Item> out;
        auto action=[&](int id,const wchar_t* fr,const wchar_t* english){out.push_back({id,en?english:fr,Kind::action,{},0,true});};
        out.push_back({1700,en?L"Enable MIDI input":L"Activer l’entrée MIDI",Kind::toggle,{},cfg_midiEnabled.get()!=0,true});
        std::vector<std::wstring> ports={en?L"No input selected":L"Aucune entrée sélectionnée"};int current=0;std::string key=cfg_midiDevice.get().get_ptr();
        for(size_t n=0;n<m_midiDevices.size();++n){ports.push_back(m_midiDevices[n].name);if(m_midiDevices[n].key==key)current=(int)n+1;}
        if(!key.empty()&&!current){ports.push_back(en?L"Saved input unavailable":L"Entrée mémorisée indisponible");current=(int)ports.size()-1;}
        out.push_back({1701,en?L"Controller input":L"Entrée du contrôleur",Kind::combo,ports,current,true});action(1702,L"Actualiser / réessayer la connexion",L"Refresh / retry connection");
        std::vector<std::wstring> names;for(int a=0;a<dj_midi::count;++a)names.push_back(dj_midi::name(a,en));out.push_back({1703,en?L"Command to assign":L"Commande à associer",Kind::combo,names,m_midiAction,true});
        out.push_back({0,dj_midi::describe(m_midiRouter.bindings[m_midiAction],en),Kind::note,{},0,true});
        out.push_back({1704,m_midiRouter.learning>=0?(en?L"Waiting: move a control or press a button…":L"En attente : bouger un contrôle ou appuyer sur un bouton…"):(en?L"MIDI Learn: assign selected command":L"MIDI Learn : associer la commande choisie"),Kind::action,{},0,m_midiInput.opened()});
        action(1705,L"Annuler l’apprentissage",L"Cancel learning");action(1706,L"Supprimer cette association",L"Remove this assignment");
        out.push_back({1707,en?L"Soft takeover for knobs / faders":L"Rattrapage des potentiomètres / faders",Kind::toggle,{},cfg_midiPickup.get()!=0,true});
        out.push_back({1708,en?L"Invert this knob / fader":L"Inverser ce potentiomètre / fader",Kind::toggle,{},m_midiRouter.bindings[m_midiAction].inverted,dj_midi::continuous(m_midiAction)&&dj_midi::valid(m_midiRouter.bindings[m_midiAction])});
        action(1709,L"Effacer les associations de cette entrée…",L"Clear this input’s assignments…");
        out.push_back({0,m_midiStatus,Kind::note,{},0,true});
        out.push_back({0,en?L"Choose input and command, then Learn. The learned message does not execute a command. CC absolute (7 bit), Pitch Bend (14 bit), Note/CC buttons only. No relative encoders, jog/scratch, SysEx or LED feedback.":L"Choisir entrée et commande puis Learn. Le message appris n’exécute pas la commande. CC absolus (7 bits), Pitch Bend (14 bits), boutons Note/CC. Pas d’encodeurs relatifs, jog/scratch, SysEx ou retour lumineux.",Kind::note,{},0,true});return out;
    }
    void CDjPanel::MidiSetting(int id,int value){
        if(id==1700){cfg_midiEnabled=value?1:0;MidiConfigure();}
        else if(id==1701){if(value==0){cfg_midiDevice="";MidiConfigure();}else if(value>0&&(size_t)value<=m_midiDevices.size()){cfg_midiDevice=m_midiDevices[value-1].key.c_str();MidiConfigure();}}
        else if(id==1702)MidiConfigure();
        else if(id==1703){m_midiAction=std::clamp(value,0,int(dj_midi::count)-1);m_midiRouter.arm(-1);}
        else if(id==1704&&m_midiInput.opened()){
            std::string key=cfg_midiDevice.get().get_ptr();bool known=false;for(auto& profile:m_midiProfiles)if(profile.device==key)known=true;
            if(!known&&m_midiProfiles.size()>=16){m_midiStatus=g_english?L"Maximum 16 saved MIDI inputs reached.":L"Limite de 16 entrées MIDI mémorisées atteinte.";return;}
            m_midiInput.clear();m_midiRouter.arm(m_midiAction);
        }
        else if(id==1705){m_midiInput.clear();m_midiRouter.arm(-1);}
        else if(id==1706){m_midiRouter.forget(m_midiAction);MidiSave();}
        else if(id==1707){cfg_midiPickup=value?1:0;m_midiRouter.pickup=value!=0;m_midiRouter.reset();}
        else if(id==1708&&dj_midi::continuous(m_midiAction)){m_midiRouter.bindings[m_midiAction].inverted=value!=0;m_midiRouter.reset();MidiSave();}
        else if(id==1709){bool en=g_english.load();if(::MessageBoxW(m_settings.m_hWnd,en?L"Clear all assignments for this MIDI input? Other inputs are preserved.":L"Effacer toutes les associations de cette entrée MIDI ? Les autres entrées sont conservées.",L"DJ Decks MIDI",MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)==IDYES){m_midiInput.clear();m_midiRouter.arm(-1);m_midiRouter.bindings={};MidiSave();}}
    }

    void CDjPanel::RefreshSettingsDevices(){
        m_settingsDevices.clear();m_settingsDeviceError.clear();
        try{auto core=output_manager::get()->getCoreConfig();output_manager_v2::get()->listDevices([&](const char* name,const GUID& backend,const GUID& device){
            if(backend==core.m_output&&device==core.m_device)return;
            m_settingsDevices.push_back({pfc::stringcvt::string_wide_from_utf8(name).get_ptr(),backend,device});
        });}catch(const std::exception& e){m_settingsDeviceError=pfc::stringcvt::string_wide_from_utf8(e.what()).get_ptr();}
    }
    void CDjPanel::OpenSettings(int page){
        if(!m_settings.m_hWnd){RefreshSettingsDevices();MidiRefreshDevices();}
        m_settings.values=[this](int tab){return SettingsValues(tab);};
        m_settings.changed=[this](int id,int value){SettingsChanged(id,value);};
        m_settings.open(m_hWnd,g_english.load(),DeckAccent(0),page);
    }
    std::vector<dj_settings::Item> CDjPanel::SettingsValues(int page){
        using namespace dj_settings;std::vector<Item> out;bool en=g_english.load();
        auto text=[&](const wchar_t* fr,const wchar_t* english){return std::wstring(en?english:fr);};
        auto combo=[&](int id,std::wstring label,std::vector<std::wstring> choices,int value){out.push_back({id,label,Kind::combo,choices,value,true});};
        auto toggle=[&](int id,std::wstring label,bool value,bool enabled=true){out.push_back({id,label,Kind::toggle,{},value?1:0,enabled});};
        auto action=[&](int id,std::wstring label){out.push_back({id,label,Kind::action,{},0,true});};
        auto note=[&](std::wstring label){out.push_back({0,label,Kind::note,{},0,true});};
        if(page==6)return MidiValues();
        if(page==0){
            combo(1000,text(L"Langue",L"Language"),{L"Français",L"English"},en?1:0);
            combo(1010,text(L"Thème",L"Theme"),{text(L"Brillant",L"Glossy"),text(L"Plat / sobre",L"Flat / subdued")},dj_skin::flat.load()?1:0);
            std::vector<std::wstring> colors={L"Cyan",L"Orange",text(L"Bleu",L"Blue"),text(L"Violet",L"Purple"),text(L"Vert",L"Green"),text(L"Rose / rouge",L"Rose / red"),text(L"Or",L"Gold"),text(L"Gris",L"Grey")};
            combo(1100,text(L"Couleur de la platine A",L"Deck A color"),colors,dj_skin::colorIndex[0].load());combo(1120,text(L"Couleur de la platine B",L"Deck B color"),colors,dj_skin::colorIndex[1].load());
            note(text(L"Les couleurs et le thème s’appliquent aux platines et à leurs playlists. Les infobulles suivent la langue choisie.",L"Decks and playlists share colors/theme. Tooltips follow the selected language."));
        }else if(page==1){
            const unsigned rates[]={0,44100,48000,96000,192000};int rateIndex=0;for(int n=0;n<5;++n)if(m_outputRatePreference.load()==rates[n])rateIndex=n;
            combo(1240,text(L"Fréquence du mix principal",L"Master sample rate"),{text(L"Auto : fréquence du premier morceau",L"Auto: first playing track"),L"44100 Hz",L"48000 Hz",L"96000 Hz",L"192000 Hz"},rateIndex);
            note(text(L"La fréquence imposée par la sortie reste prioritaire. Auto suit le premier morceau au démarrage du moteur, puis reste fixe. Une modification s’applique au prochain redémarrage de foobar.",L"A rate forced by the output has priority. Auto follows the first track at engine start, then remains fixed. Changes apply after restarting foobar."));
            combo(1230,text(L"Analyseur BPM",L"BPM analyser"),{text(L"Automatique : Smart Tempo puis foo_bpm",L"Auto: Smart Tempo, then foo_bpm"),L"foo_smart_tempo",L"foo_bpm"},std::clamp(int(cfg_bpmProvider.get()),0,2));
            note(text(L"Analyse externe facultative sur le morceau chargé. Conserver BPM comme nom du tag dans l’analyseur. Valider/enregistrer le résultat puis Actualiser les tags ; reprendre la lecture manuellement.",L"Optional external analysis of the loaded track. Keep BPM as the analyser's output tag. Save the result, then Refresh tags; resume playback manually."));
            toggle(1220,text(L"Limiteur du mix principal",L"Master output limiter"),m_limitEnabled.load());
            const int ceilings[]={-3,-10,-30,-60};int ceiling=4;std::vector<std::wstring> choices={L"-0.3 dBFS",L"-1 dBFS",L"-3 dBFS",L"-6 dBFS"};for(int n=0;n<4;++n)if(cfg_masterCeiling.get()==ceilings[n])ceiling=n;if(ceiling==4)choices.push_back(std::to_wstring(cfg_masterCeiling.get()/10.)+L" dBFS");combo(1221,text(L"Plafond du limiteur",L"Limiter ceiling"),choices,ceiling);
            note(text(L"Voyant du mix : vert = normal, orange = réduction (dB), rouge = saturation sans limiteur ou signal invalide. Stéréo liée, relâchement 80 ms, sans latence ajoutée. Crêtes échantillonnées, pas true peak ; préécoute casque indépendante.",L"Master indicator: green = normal, amber = reduction (dB), red = clipping with limiter bypassed or invalid signal. Stereo linked, 80 ms release, no added latency. Sample peaks, not true peak; headphone cue is independent."));
            combo(1210,L"ReplayGain",{text(L"Désactivé",L"Off"),text(L"Piste : gain + crête",L"Track: gain + peak"),text(L"Album : gain + crête",L"Album: gain + peak")},g_replaygain.load());
            toggle(220,text(L"Scrobbler les platines audibles via foo_scrobble",L"Scrobble audible decks via foo_scrobble"),g_scrobble.load());action(221,text(L"Diagnostic foo_scrobble…",L"foo_scrobble diagnostic…"));
            note(text(L"Le scrobbling nécessite foo_scrobble 1.6.x configuré. ReplayGain ne modifie pas les tags. La sortie du mix principal reste celle configurée dans foobar.",L"Scrobbling requires configured foo_scrobble 1.6.x. ReplayGain does not edit tags. Master output follows foobar's output configuration."));
        }else if(page==2){
            std::vector<std::wstring> devices={text(L"Aucune sortie sélectionnée",L"No output selected")};int selected=0;
            for(size_t n=0;n<m_settingsDevices.size();++n){auto& d=m_settingsDevices[n];devices.push_back(d.name);if(d.backend==cfg_cueBackend.get()&&d.device==cfg_cueDevice.get())selected=(int)n+1;}
            if(cfg_cueBackend.get()!=GUID{}&&!selected){devices.push_back(text(L"Sortie mémorisée indisponible / mix principal",L"Saved output unavailable / master"));selected=(int)devices.size()-1;}
            combo(1400,text(L"Sortie casque",L"Headphone output"),devices,selected);action(1401,text(L"Actualiser les sorties disponibles",L"Refresh available outputs"));
            std::vector<std::wstring> volume;for(int n=0;n<=100;++n)volume.push_back(std::to_wstring(n)+L" %");combo(1402,text(L"Volume casque",L"Headphone volume"),volume,std::clamp((int)cfg_cueVolume.get(),0,100));
            std::vector<std::wstring> previews;for(int n=0;n<5;++n)previews.push_back(dj_preview::name(n,en));combo(1403,text(L"Extraits de playlist",L"Playlist excerpts"),previews,std::clamp((int)cfg_previewMode.get(),0,4));
            toggle(1404,L"Cue A",m_cue[0].load(),cfg_cueBackend.get()!=GUID{});toggle(1405,L"Cue B",m_cue[1].load(),cfg_cueBackend.get()!=GUID{});
            action(1406,text(L"Arrêter l’extrait de playlist",L"Stop playlist excerpt"));action(1407,text(L"Désactiver Cue A/B et libérer la sortie",L"Disable Cue A/B and release output"));action(1408,text(L"Réessayer la sortie mémorisée",L"Retry saved output"));
            auto error=m_cueOutput.lastError();if(!error.empty())note(pfc::stringcvt::string_wide_from_utf8(error.c_str()).get_ptr());if(!m_settingsDeviceError.empty())note(m_settingsDeviceError);
            note(text(L"Cue A/B suit la lecture de chaque deck. Les extraits des playlists sont indépendants. Le mix principal est exclu des sorties proposées. Bluetooth : latence possible.",L"Cue A/B follows deck transport; playlist excerpts are independent. Master output is excluded. Bluetooth latency is possible."));
        }else if(page==3){
            std::vector<std::wstring> formats={text(L"Artiste - Titre (durée)",L"Artist - Title (duration)"),text(L"Titre - Artiste (durée)",L"Title - Artist (duration)"),text(L"Album | N° de piste. Artiste - Titre (durée)",L"Album | Track no. Artist - Title (duration)"),text(L"Artiste - Titre",L"Artist - Title"),text(L"Personnalisé…",L"Custom…")};
            int selected=4;for(int n=0;n<4;++n)if(std::string(cfg_queueFormat.get().get_ptr())==dj_queue_format::presets[n])selected=n;combo(1300,text(L"Format d’affichage",L"Display format"),formats,selected);
            action(304,text(L"Modifier le format personnalisé…",L"Edit custom format…"));toggle(305,text(L"Afficher les numéros de position",L"Show playlist position numbers"),cfg_queueNumbers.get()!=0);action(306,text(L"Relire les métadonnées des deux playlists",L"Reload both playlists' metadata"));
            note(pfc::stringcvt::string_wide_from_utf8(cfg_queueFormat.get().get_ptr()).get_ptr());
            action(1604,text(L"Répartir la playlist active de foobar : A / B…",L"Distribute active foobar playlist: A / B…"));
            action(1600,text(L"Enregistrer les deux playlists dans un fichier…",L"Save both playlists to a file…"));action(1601,text(L"Enregistrer une session complète (playlists + platines)…",L"Save a complete session (playlists + decks)…"));action(1602,text(L"Charger une sauvegarde…",L"Load a saved session…"));action(1603,text(L"Revenir avant le dernier chargement ou répartition…",L"Restore before the last import or distribution…"));
            note(text(L"Fichiers JSON : aucun morceau audio n’est copié. La session complète conserve positions, points A/B, grilles manuelles, vitesse, tonalité et mixage. Au chargement : lecture, boucles et AUTO désactivés.",L"JSON files reference audio without copying it. Complete sessions include positions, A/B points, manual grids, speed, pitch and mixer settings. Full loads restore paused, with loops and AUTO off."));
        }else if(page==4){
            std::vector<std::wstring> duration;int selected=0;const int seconds[]={0,1,2,3,5,8,10,15,20,30};for(int n=0;n<10;++n){duration.push_back(std::to_wstring(seconds[n])+L" s");if(m_fadeSeconds==seconds[n])selected=n;}
            if(m_fadeSeconds!=seconds[selected]){duration.push_back(std::to_wstring(m_fadeSeconds)+L" s");selected=10;}
            combo(1500,text(L"Durée du fondu A/B",L"A/B fade duration"),duration,selected);combo(1501,text(L"Courbe du crossfader",L"Crossfader curve"),{text(L"Puissance constante",L"Equal power"),text(L"Linéaire",L"Linear")},m_linearFade.load()?1:0);
            toggle(1502,text(L"Q : caler les points A/B de la platine A",L"Q: snap A/B points on deck A"),m_loopSnap[0]);toggle(1503,text(L"Q : caler les points A/B de la platine B",L"Q: snap A/B points on deck B"),m_loopSnap[1]);
            toggle(1510,text(L"Supprimer le silence au début",L"Trim leading silence"),cfg_trimHead.get()!=0);toggle(1511,text(L"Supprimer le silence à la fin",L"Trim trailing silence"),cfg_trimTail.get()!=0);
            const int levels[]={-96,-90,-80,-70,-60,-50,-40,-30};std::vector<std::wstring> thresholds;selected=-1;for(int n=0;n<8;++n){thresholds.push_back(std::to_wstring(levels[n])+L" dBFS");if(cfg_trimDb.get()==levels[n])selected=n;}
            if(selected<0){thresholds.push_back(std::to_wstring(cfg_trimDb.get())+L" dBFS");selected=8;}combo(1512,text(L"Seuil de silence",L"Silence threshold"),thresholds,selected);
            note(text(L"Silences : appliqués aux prochains chargements, avec préchargement du suivant en AUTO. Activer Q recale aussi les points existants sur la grille affichée. Choisir le mode AUTO sur chaque deck.",L"Silence options apply to new loads; AUTO preloads the next track. Enabling Q also snaps existing points to the displayed grid. Select AUTO modes on each deck."));
        }else if(page==5){action(200,text(L"Taille et nettoyage du cache des stems…",L"Stem cache size and cleanup…"));note(text(L"Les stems restent expérimentaux. Le gestionnaire affiche la taille et permet de vider les entrées inutilisées en conservant celles qui sont actives ou en préparation. Les morceaux source sont préservés.",L"Stems remain experimental. The cache manager shows size and removes unused entries while retaining active/preparing stems. Source music is preserved."));note(text(L"Moteur local facultatif : accélération NVIDIA CUDA automatique, sinon CPU. Préparation, volumes et solos restent accessibles sur chaque deck.",L"Optional local engine: automatic NVIDIA CUDA acceleration with CPU fallback. Preparation, volume and solo controls remain on each deck."));}
        if(page>=0&&page<5)action(1900+page,text(L"Rétablir les valeurs par défaut de cette catégorie…",L"Reset this category to defaults…"));
        return out;
    }
    void CDjPanel::SettingsChanged(int id,int value){
        if(id>=1700&&id<=1709){MidiSetting(id,value);return;}
        if(id>=1900&&id<1905){
            // Reuse the normal setters so persisted and live values stay in sync.
            // Language, loaded media, transport and loop points are intentionally retained.
            const std::vector<std::vector<std::pair<int,int>>> defaults={
                {{1010,0},{1100,0},{1120,1}},
                {{1240,0},{1230,0},{1220,1},{1221,1},{1210,0},{220,0}},
                {{1400,0},{1402,25},{1403,0}},
                {{1300,0},{305,0}},
                {{1500,4},{1501,0},{1502,0},{1503,0},{1510,0},{1511,0},{1512,4}}
            };
            for(const auto& setting:defaults[id-1900])SettingsChanged(setting.first,setting.second);
            return;
        }
        if(id==1604)DistributeActivePlaylist();else if(id==1600||id==1601)ExportBundle(id==1601);else if(id==1602||id==1603)ImportBundle(id==1603);
        else if(id==1000)SettingsCommand(1+value);else if(id==1010)SettingsCommand(10+value);else if(id==1100)SettingsCommand(100+value);else if(id==1120)SettingsCommand(120+value);else if(id==1210)SettingsCommand(210+value);else if(id==1300)SettingsCommand(value==4?304:300+value);
        else if(id==220){if((value!=0)!=g_scrobble.load())SettingsCommand(220);}else if(id==305){if((value!=0)!=(cfg_queueNumbers.get()!=0))SettingsCommand(305);}
        else if(id==200||id==221||id==304||id==306)SettingsCommand(id);
        else if(id==1240){const unsigned rates[]={0,44100,48000,96000,192000};if(value>=0&&value<5){cfg_outputRate=rates[value];m_outputRatePreference=rates[value];}}
        else if(id==1230){cfg_bpmProvider=std::clamp(value,0,2);}
        else if(id==1220){m_limitEnabled=value!=0;cfg_masterLimiter=value?1:0;}
        else if(id==1221){const int ceilings[]={-3,-10,-30,-60};if(value>=0&&value<4){cfg_masterCeiling=ceilings[value];m_limitCeiling=ceilings[value]/10.;}}
        else if(id==1400){
            if(value==0){StopPreview();m_cue[0]=m_cue[1]=false;cfg_cueBackend=GUID{};cfg_cueDevice=GUID{};ConfigureCue();}
            else if(value>0&&(size_t)value<=m_settingsDevices.size()){auto d=m_settingsDevices[value-1];auto core=output_manager::get()->getCoreConfig();if(d.backend==core.m_output&&d.device==core.m_device){RefreshSettingsDevices();return;}StopPreview();cfg_cueBackend=d.backend;cfg_cueDevice=d.device;ConfigureCue();}
        }else if(id==1401)RefreshSettingsDevices();else if(id==1402){cfg_cueVolume=std::clamp(value,0,100);m_cueVolume=cfg_cueVolume.get()/100.;}
        else if(id==1403)cfg_previewMode=std::clamp(value,0,4);
        else if(id==1404||id==1405){if(cfg_cueBackend.get()!=GUID{}){StopPreview();m_cue[id-1404]=value!=0;ConfigureCue();}}
        else if(id==1406)StopPreview();else if(id==1407){StopPreview();m_cue[0]=m_cue[1]=false;ConfigureCue();}else if(id==1408){StopPreview();ConfigureCue();}
        else if(id==1500){const int seconds[]={0,1,2,3,5,8,10,15,20,30};if(value>=0&&value<10){m_fadeSeconds=seconds[value];SaveSession();}}
        else if(id==1501){m_linearFade=value==1;SaveSession();}
        else if(id==1502||id==1503)SetLoopSnap(id-1502,value!=0);
        else if(id==1510||id==1511||id==1512){if(id==1510)cfg_trimHead=value?1:0;if(id==1511)cfg_trimTail=value?1:0;if(id==1512){const int levels[]={-96,-90,-80,-70,-60,-50,-40,-30};if(value>=0&&value<8)cfg_trimDb=levels[value];}RefreshTrimPreparations();}
        m_settings.english=g_english.load();m_settings.accent=DeckAccent(0);Invalidate();
    }

    void CDjPanel::SilenceMenu(CPoint point){
        bool en=g_english.load();auto options=TrimOptions();HMENU menu=CreatePopupMenu(),thresholds=CreatePopupMenu();
        AppendMenuW(menu,MF_STRING|(options.head?MF_CHECKED:0),1,en?L"Skip leading silence":L"Supprimer le silence au début");
        AppendMenuW(menu,MF_STRING|(options.tail?MF_CHECKED:0),2,en?L"Skip trailing silence":L"Supprimer le silence à la fin");
        const int values[]={-96,-90,-80,-70,-60,-50,-40,-30};for(int n=0;n<8;++n){auto label=std::to_wstring(values[n])+L" dBFS";AppendMenuW(thresholds,MF_STRING|(options.db==values[n]?MF_CHECKED:0),100+n,label.c_str());}
        AppendMenuW(menu,MF_POPUP,(UINT_PTR)thresholds,en?L"Silence threshold":L"Seuil de silence");
        AppendMenuW(menu,MF_SEPARATOR,0,nullptr);AppendMenuW(menu,MF_STRING|MF_GRAYED,0,en?L"Applies to newly loaded tracks · next AUTO track preloaded":L"Prochains chargements · préchargement du suivant en AUTO");
        ClientToScreen(&point);auto choice=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY,point.x,point.y,0,m_hWnd,nullptr);DestroyMenu(menu);
        if(!choice)return;if(choice==1)cfg_trimHead=!options.head;if(choice==2)cfg_trimTail=!options.tail;if(choice>=100&&choice<108)cfg_trimDb=values[choice-100];
        RefreshTrimPreparations();
    }

    void CDjPanel::CueMenu(CPoint point){
        const bool en=g_english.load();
        struct Device{std::wstring name;GUID backend,device;};std::vector<Device> devices;
        try{output_manager_v2::get()->listDevices([&](const char* name,const GUID& backend,const GUID& device){devices.push_back({pfc::stringcvt::string_wide_from_utf8(name).get_ptr(),backend,device});});}
        catch(const std::exception& e){auto text=pfc::stringcvt::string_wide_from_utf8(e.what());::MessageBoxW(m_hWnd,text.get_ptr(),L"DJ Decks - Cue",MB_OK|MB_ICONERROR);return;}
        HMENU menu=CreatePopupMenu(),outputs=CreatePopupMenu(),volume=CreatePopupMenu(),excerpts=CreatePopupMenu();
        auto error=m_cueOutput.lastError();if(!error.empty()){auto text=pfc::stringcvt::string_wide_from_utf8(error.c_str());AppendMenuW(menu,MF_STRING|MF_GRAYED,0,text.get_ptr());}
        AppendMenuW(menu,MF_STRING,1,en?L"Disable cue / release device":L"Désactiver la préécoute / libérer la sortie");
        auto core=output_manager::get()->getCoreConfig();
        for(size_t n=0;n<devices.size()&&n<1000;++n){auto& d=devices[n];bool master=d.backend==core.m_output&&d.device==core.m_device;
            auto name=d.name;if(master)name+=en?L" (master)":L" (mix principal)";
            AppendMenuW(outputs,MF_STRING|(master?MF_GRAYED:0)|((d.backend==cfg_cueBackend.get()&&d.device==cfg_cueDevice.get())?MF_CHECKED:0),100+n,name.c_str());}
        AppendMenuW(menu,MF_POPUP,(UINT_PTR)outputs,en?L"Headphone output":L"Sortie casque");
        for(int v:{0,10,25,50,75,100}){auto label=std::to_wstring(v)+L" %";AppendMenuW(volume,MF_STRING|(cfg_cueVolume.get()==v?MF_CHECKED:0),2000+v,label.c_str());}
        AppendMenuW(menu,MF_POPUP,(UINT_PTR)volume,en?L"Cue volume":L"Volume casque");
        for(int mode=0;mode<5;++mode)AppendMenuW(excerpts,MF_STRING|(cfg_previewMode.get()==mode?MF_CHECKED:0),3000+mode,dj_preview::name(mode,en));
        AppendMenuW(menu,MF_POPUP,(UINT_PTR)excerpts,en?L"Playlist excerpt":L"Extrait depuis les playlists");
        AppendMenuW(menu,MF_STRING,5,en?L"Stop playlist excerpt":L"Arrêter l’extrait de playlist");
        for(int i=0;i<2;++i)AppendMenuW(menu,MF_STRING|(m_cue[i].load()?MF_CHECKED:0),2+i,i?L"Cue B":L"Cue A");
        AppendMenuW(menu,MF_STRING,4,en?L"Retry selected output":L"Réessayer la sortie sélectionnée");
        AppendMenuW(menu,MF_SEPARATOR,0,nullptr);AppendMenuW(menu,MF_STRING|MF_GRAYED,0,en?L"Bluetooth / separate devices: latency and drift possible":L"Bluetooth / sorties séparées : latence et dérive possibles");
        ClientToScreen(&point);auto choice=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY,point.x,point.y,0,m_hWnd,nullptr);DestroyMenu(menu);
        if(choice>=3000&&choice<3005)cfg_previewMode=choice-3000;
        if(choice==5)StopPreview();
        if(choice==1){StopPreview();m_cue[0]=m_cue[1]=false;ConfigureCue();}
        if(choice==2||choice==3){if(cfg_cueBackend.get()!=GUID{}){StopPreview();int i=choice-2;m_cue[i]=!m_cue[i].load();ConfigureCue();}}
        if(choice==4){StopPreview();ConfigureCue();}
        if(choice>=100&&choice<1100&&(size_t)(choice-100)<devices.size()){auto& d=devices[choice-100];StopPreview();cfg_cueBackend=d.backend;cfg_cueDevice=d.device;ConfigureCue();}
        if(choice>=2000&&choice<=2100){cfg_cueVolume=choice-2000;m_cueVolume=(choice-2000)/100.;}
    }

	// Le moteur : decode, mixe et envoie le son a la sortie audio de foobar2000.
	void CDjPanel::EngineThread() {
		try {
			// Sortie audio configuree dans foobar2000, avec un petit tampon (0,1 s) pour une bonne reactivite.
			output::ptr out = output_manager::get()->instantiateCoreDefault(0.1);

			// Frequence imposee par la sortie (sinon 48000 Hz). Elle ne changera plus jamais ensuite.
			const unsigned forcedRate = out->get_forced_sample_rate_();
			unsigned sourceRate=0;
            {std::lock_guard<std::mutex> lock(m_lock);for(auto& deck:m_deck)if(deck.playing&&deck.sourceRate){sourceRate=deck.sourceRate;break;}if(!sourceRate)for(auto& deck:m_deck)if(deck.loaded&&deck.sourceRate){sourceRate=deck.sourceRate;break;}}
            const unsigned outRate = dj_output::rate(forcedRate,m_outputRatePreference.load(),sourceRate);
			{
				std::lock_guard<std::mutex> lock(m_lock);
                const unsigned previousRate=m_outRate?m_outRate:48000;
                for(auto& deck:m_deck)deck.framesPlayed*=double(outRate)/previousRate;
				m_outRate = outRate;
			}

			abort_callback_dummy abort;
			std::vector<audio_sample> mix,cueMix;
            double previousCue[2]={0,0};
            dj_master::Limiter masterLimiter;
			ULONGLONG notReadySince = 0;   // moment ou la sortie a cesse d'etre prete (0 = elle l'est)
			bool stallLogged = false;
			FB2K_console_formatter() << "foo_dj : moteur audio demarre, sortie a " << outRate << " Hz";

			// Gains des deux decks au bloc precedent (pour les faire evoluer en douceur).
			// Gain d'un deck = gain du crossfader (puissance constante) x volume au carre.
			auto computeGains = [this](double gains[2]) {
				const double x = (std::min)(1.0, (std::max)(0.0, (double)m_crossfade.load()));
				const double v0 = (std::min)(1.0, (std::max)(0.0, (double)m_volume[0].load()));
				const double v1 = (std::min)(1.0, (std::max)(0.0, (double)m_volume[1].load()));
				dj_auto::gains(x,m_linearFade.load(),gains);gains[0]*=v0*v0;gains[1]*=v1*v1;
                for(int i=0;i<2;++i)gains[i]*=dj_listening::gain(g_replaygain.load(),m_rgTrack[i].load(),m_rgAlbum[i].load());
			};
			double prevGain[2];
			computeGains(prevGain);

			while (!m_stop) {
				bool ready = false;
				out->update(ready);
				if (!ready) {
					if (!notReadySince) notReadySince = GetTickCount64();
					const ULONGLONG waited = GetTickCount64() - notReadySince;
					m_stallMs = (int)waited;
					if (waited > 2000 && !stallLogged) {
						stallLogged = true;
						FB2K_console_formatter() << "foo_dj : la sortie audio n'est plus prete depuis 2 secondes";
					}
					Sleep(2);
					continue;
				}
				notReadySince = 0;
				m_stallMs = 0;
				stallLogged = false;

				double targetGain[2];
				computeGains(targetGain);

                const size_t cueToken=m_cueOutput.token();
                const bool cueA=m_cue[0].load(),cueB=m_cue[1].load();
                double cueGain[2];dj_cue::gains(cueA,cueB,m_cueVolume.load(),cueGain);
                for(int i=0;i<2;++i)cueGain[i]*=dj_listening::gain(g_replaygain.load(),m_rgTrack[i].load(),m_rgAlbum[i].load());
                cueMix.clear();
                const double speeds[2] = {m_speed[0].load(), m_speed[1].load()};
				const bool fixedPitch[2] = {m_fixedPitch[0].load(), m_fixedPitch[1].load()};
				size_t frames = 0;
				{
					std::lock_guard<std::mutex> lock(m_lock);

					// 1. Decoder de quoi remplir un bloc pour chaque deck en lecture
					for (int i = 0; i < 2; ++i) {
						Deck & d = m_deck[i];
						if (!d.playing) continue;
						try {
							dj_loop::wrap(d,outRate,abort);
                            FillDeck(d, kBlockFrames, outRate, abort, speeds[i], fixedPitch[i], m_transpose[i].load());
						} catch (const std::exception & e) {
							pfc::stringcvt::string_wide_from_utf8 message(e.what());
							d.playing = false; d.loop.enabled=false;
							d.eof = true;
							m_error[i] = std::wstring(L"Erreur : ") + message.get_ptr();
						}
					}

					// 2. Combien d'images a envoyer : celles du deck le plus fourni
					for (int i = 0; i < 2; ++i) {
						const Deck & d = m_deck[i];
						if (d.playing && d.pendingFrames() > 0) {
							auto available=(std::min)(kBlockFrames,d.pendingFrames());
                                if(d.trimEnd>0&&d.trimEnd<d.length)available=dj_silence::cap(d.trimEnd,d.framesPlayed/outRate,speeds[i],outRate,available);
                                frames=(std::max)(frames,available);
						}
					}

					// Split the common output block at either loop boundary so the other deck keeps its clock.
                    for(int i=0;i<2;++i)if(m_deck[i].playing)frames=m_deck[i].loop.cap(m_deck[i].framesPlayed/outRate,speeds[i],outRate,frames);

                    // The fade clock follows produced audio, so output stalls and pause cannot run it ahead.
                    if(m_autoFade.active && frames>0 && m_deck[m_autoFade.to].playing) {
                        m_crossfade=(float)m_autoFade.advance(frames,true);
                    }
                    computeGains(targetGain);
                    // 3. Mixage : chaque deck est multiplie par son gain (crossfader x volume).
					// Le gain evolue en douceur du bloc precedent vers le nouveau (pas de craquements).
					if (frames > 0) {
						mix.assign(frames * kOutChannels, (audio_sample)0);
                        cueMix.assign(frames*kOutChannels,(audio_sample)0);
						for (int i = 0; i < 2; ++i) {
							Deck & d = m_deck[i];
							if (d.playing && d.pendingFrames() > 0) {
                                size_t f=(std::min)(frames,d.pendingFrames());
                                if(d.trimEnd>0&&d.trimEnd<d.length)f=dj_silence::cap(d.trimEnd,d.framesPlayed/outRate,speeds[i],outRate,f);
								for (size_t n = 0; n < f; ++n) {
									const audio_sample g = (audio_sample)(prevGain[i] + (targetGain[i] - prevGain[i]) * (double)(n + 1) / (double)f);
                                    const audio_sample cg=(audio_sample)(previousCue[i]+(cueGain[i]-previousCue[i])*(double)(n+1)/(double)f);
                                    dj_cue::mixFrame(d.pending[d.pendingPos+n*2],d.pending[d.pendingPos+n*2+1],g,cg,&mix[n*2],&cueMix[n*2]);
								}
								d.pendingPos += f * kOutChannels;
								d.framesPlayed += f * speeds[i];
                                bool audibleStem=!d.stems; if(d.stems)for(int k=0;k<4;++k)audibleStem=audibleStem||d.stems->target(k)>0;
                                d.listening.consume(f,outRate,audibleStem?targetGain[i]:0.,g_scrobble.load()&&g_scrobbleReady.load()&&m_scrobbleAllowed[i].load(),(long long)std::time(nullptr));
                                if(!d.loop.enabled&&d.trimEnd>0&&d.trimEnd<d.length&&d.framesPlayed/outRate>=d.trimEnd){d.eof=true;d.playing=false;d.pending.clear();d.pendingPos=0;d.src.clear();d.stretch.reset();}
							}
						}
					}

					if(m_autoFade.finished)m_deck[m_autoFade.from].playing=false;
                    // 4. Menage : decks arrives au bout, tampons vides
					for (int i = 0; i < 2; ++i) {
						Deck & d = m_deck[i];
						if (d.playing && !d.loop.enabled && d.eof && d.pendingFrames() == 0) d.playing = false;
						if (d.pendingPos == d.pending.size()) { d.pending.clear(); d.pendingPos = 0; }
						else if (d.pendingPos >= 32768) { d.pending.erase(d.pending.begin(), d.pending.begin() + d.pendingPos); d.pendingPos = 0; }
					}
				}

				if (frames == 0) {
					// Aucun deck ne joue : on envoie du silence pour que la sortie ne s'arrete jamais.
					frames = kBlockFrames;
					mix.assign(frames * kOutChannels, (audio_sample)0);
				}
                // Post-fader/volume/ReplayGain mix only. Never process cueMix here.
                const bool limitEnabled=m_limitEnabled.load();
                const auto meter=masterLimiter.process(mix.data(),frames,outRate,limitEnabled,m_limitCeiling.load());
                m_masterPeak=meter.output;const auto meterNow=GetTickCount64();
                if(meter.invalid||(!limitEnabled&&meter.input>1.))m_masterClipUntil=meterNow+1000;
                if(limitEnabled&&meter.reduction>.1){m_limitReduction=meter.reduction;m_masterLimitUntil=meterNow+350;}
				audio_chunk_impl chunk;
				chunk.set_data(mix.data(), frames, kOutChannels, outRate, audio_chunk::channel_config_stereo);
                out->process_samples(chunk);
                if(!m_previewActive.load()&&(cueA||cueB)){if(cueMix.empty())cueMix.assign(frames*2,(audio_sample)0);m_cueOutput.push(std::move(cueMix),outRate,cueToken);}
                previousCue[0]=cueGain[0];previousCue[1]=cueGain[1];
				prevGain[0] = targetGain[0];
				prevGain[1] = targetGain[1];
			}
		} catch (const std::exception & e) {
			pfc::stringcvt::string_wide_from_utf8 message(e.what());
			FB2K_console_formatter() << "foo_dj : erreur du moteur audio : " << e.what();
			std::lock_guard<std::mutex> lock(m_lock);
			for (int i = 0; i < 2; ++i) {
				m_deck[i].playing = false;
				m_error[i] = std::wstring(L"Sortie audio : ") + message.get_ptr();
			}
		} catch (...) {
			std::lock_guard<std::mutex> lock(m_lock);
			for (int i = 0; i < 2; ++i) {
				m_deck[i].playing = false;
				m_error[i] = L"Sortie audio : erreur inconnue";
			}
		}
		m_stallMs = 0;
		m_masterPeak=0;m_limitReduction=0;m_masterClipUntil=0;m_masterLimitUntil=0;
		m_engineFinished = true;
	}

	// ui_element_impl_withpopup genere aussi une version "fenetre flottante" et les commandes de menu.
	class ui_element_djimpl : public ui_element_impl_withpopup<CDjPanel> {};

	static service_factory_single_t<ui_element_djimpl> g_ui_element_djimpl_factory;

} // namespace
