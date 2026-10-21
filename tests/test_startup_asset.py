"""Check the editable cue and the exact committed MIDI/WAV/Flash representations."""
import importlib.util
from pathlib import Path
import re
import struct
import unittest
import wave

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("gen_startup_sound", ROOT / "tools/gen_startup_sound.py")
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class StartupAssetTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.score = generator.load_score()
        cls.samples = generator.render(cls.score)

    def test_pcm_headroom_and_tail(self):
        rate = self.score["sample_rate"]
        self.assertEqual(len(self.samples), rate * self.score["duration_ms"] // 1000)
        self.assertEqual(max(map(abs, self.samples)), 11600)
        lead = min(note["start_ms"] for note in self.score["notes"]) * rate // 1000
        self.assertTrue(all(x == 0 for x in self.samples[:lead]))
        self.assertTrue(all(x == 0 for x in self.samples[-rate * 120 // 1000:]))
        self.assertGreater(sum(x * x for x in self.samples) / len(self.samples), 100000)
        self.assertLess(max(abs(a - b) for a, b in zip(self.samples, self.samples[1:])), 8000)

    def test_wav_and_flash_match_score(self):
        with wave.open(str(ROOT / "assets/music/startup.wav"), "rb") as wav:
            self.assertEqual((wav.getnchannels(), wav.getsampwidth(), wav.getframerate()), (1, 2, 16000))
            self.assertEqual(wav.readframes(wav.getnframes()), struct.pack(f"<{len(self.samples)}h", *self.samples))
        header = (ROOT / "main/startup_sound_data.h").read_text(encoding="utf-8")
        self.assertEqual(header, generator.header_text(self.score, self.samples))
        values = list(map(int, re.findall(r"-?\d+", header.split("= {", 1)[1])))
        self.assertEqual(values, self.samples)

    def test_midi_is_editable_and_balanced(self):
        midi = generator.midi_bytes(self.score)
        self.assertEqual(midi, (ROOT / "assets/music/startup.mid").read_bytes())
        self.assertEqual(midi[:14], b"MThd\x00\x00\x00\x06\x00\x00\x00\x01\x01\xe0")
        self.assertEqual(len(midi) - 22, int.from_bytes(midi[18:22], "big"))
        self.assertIn(bytes((0xC0, self.score["program"])), midi)
        for note in self.score["notes"]:
            self.assertIn(bytes((0x90, note["midi"], note["velocity"])), midi)
            self.assertIn(bytes((0x80, note["midi"], 0)), midi)
        self.assertTrue(midi.endswith(b"\xff\x2f\x00"))


if __name__ == "__main__":
    unittest.main()
