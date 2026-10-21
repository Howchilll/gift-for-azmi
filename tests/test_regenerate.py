#!/usr/bin/env python3
"""Reject DIY edits that would silently drop items or use missing glyphs."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('regenerate', ROOT / 'tools/regenerate.py')
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class EditableProfileTest(unittest.TestCase):
    def setUp(self):
        self.profile = json.loads((ROOT / 'data/profile.json').read_text(encoding='utf-8'))

    def test_included_profile_is_valid(self):
        MODULE.validate(self.profile)

    def test_extra_items_are_not_silently_discarded(self):
        items = self.profile['journeyPages'][0]['items']
        items.append(copy.deepcopy(items[0]))
        with self.assertRaisesRegex(ValueError, 'one or two items'):
            MODULE.validate(self.profile)

    def test_unsupported_font_characters_are_explained(self):
        self.profile['name'] = '\u8001\u5e08'
        with self.assertRaisesRegex(ValueError, 'font supports'):
            MODULE.validate(self.profile)

    def test_invalid_url_fails_before_generation(self):
        self.profile['profileUrl'] = 'not a URL'
        with self.assertRaisesRegex(ValueError, 'http'):
            MODULE.validate(self.profile)


if __name__ == '__main__':
    unittest.main()
