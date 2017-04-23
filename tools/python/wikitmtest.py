#!/usr/bin/python
import unittest
import wikitm as wt

import datetime

TEST_DUMP = """<mediawiki>
<page>
	<title>1st Article</title>
	<revision>
		<timestamp>2001-12-02T15:08:12Z</timestamp>
		<text> 1st revision </text>
	</revision>
	<revision>
		<timestamp>2001-12-02T15:40:00Z</timestamp>
		<text> 2nd revision </text>
	</revision>
	<revision>
		<timestamp>2002-12-02T15:40:00Z</timestamp>
		<text> 3rd revision </text>
	</revision>
</page>
<page>
	<title>2nd Article</title>
	<revision>
		<timestamp>2001-12-02T15:08:12Z</timestamp>
		<text> 1st revision </text>
	</revision>
	<revision>
		<timestamp>2001-12-02T15:40:00Z</timestamp>
		<text> 2nd revision </text>
	</revision>
	<revision>
		<timestamp>2002-12-02T15:40:00Z</timestamp>
		<text> 3rd revision </text>
	</revision>
</page>
</mediawiki>"""



class TestWikiTMMethods(unittest.TestCase) :

    def test_find_pages_1a(self):
        pages, trail = wt.find_pages(TEST_DUMP)
        self.assertEqual(len(pages), 2)

    def test_find_pages_1b(self):
        pages, trail = wt.find_pages(TEST_DUMP[18:])
        self.assertEqual(len(pages), 1)

    def test_find_pages_2a(self):
        pages, trail = wt.find_pages(TEST_DUMP)
        self.assertEqual(trail, "\n</mediawiki>")

    def test_find_pages_2b(self):
        pages, trail = wt.find_pages(TEST_DUMP[len(TEST_DUMP)-20:])
        self.assertEqual(trail, "\n</mediawiki>")


    def test_gen_timeline_1(self):
        delta = datetime.timedelta(days=90)
        startdate = datetime.datetime.today()
        count = 5
        timelines = wt.gen_timeline(startdate, delta, count)
        self.assertEqual(count, len(timelines))


    def test_gen_timeline_2(self):
        delta = datetime.timedelta(days=90)
        startdate = datetime.datetime.today()
        count = 5
        timelines = wt.gen_timeline(startdate, delta, count)
        self.assertEqual( timelines[1], startdate + delta)


    def test_gen_dumplist(self):
        path = "/media/datahd/dumps.wikimedia.org/enwiki/20170101-uncompressed/"
        dumplist = wt.gen_dumplist(path)
        self.assertEqual(len(dumplist), 78)

if __name__ == "__main__":
    unittest.main()
