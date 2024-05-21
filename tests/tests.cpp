#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "rgbimage.hpp"
#include "textrenderer.hpp"
#include "stream.hpp"
#ifdef __linux__
#include "v4ldev.hpp"
#endif
#include "frameserver.hpp"

#include <unistd.h>

#include <gst/gst.h>

#include <gtest/gtest.h>
#include "httpserver.hpp"


TEST (testpaste, subtest) {
	rgbimage test(800,600);
	rgbimage test_paste(400,300);

	test_paste.setcolor(0xFF, 0x00, 0x00);
	test.paste(&test_paste, 0, 0);
	test.paste(&test_paste, 400, 300);
	test.paste(&test_paste, 400, 0);
	test.paste(&test_paste, 0, 300);

    ASSERT_EQ(test.get_h(), 600);
}

TEST (testautocrop, subtest) {
    rgbimage temp = rgbimage(800, 600);
    uint8_t white[3] = { 0xFF, 0xFF, 0xFF };
    temp.setcolor(0x00, 0x00, 0x00);

    temp.putpixel(34, 20, white);
    temp.putpixel(440, 40, white);
    temp.putpixel(40, 540, white);
    temp.autocrop();

    ASSERT_EQ(temp.get_h(), 521);
    ASSERT_EQ(temp.get_w(), 440-34+1);
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}


