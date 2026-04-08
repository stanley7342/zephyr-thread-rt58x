/*
 * RT583 Flash Driver Unit Test
 *
 * 測試區域：storage_partition（label "storage"，0x190000，128 KB）
 *
 * 測試分組：
 *   suite_basic   — 基本讀寫擦除
 *   suite_pattern — 資料 pattern 驗證
 *   suite_boundary— 邊界 / 跨界情境
 *   suite_error   — 錯誤參數處理
 *   suite_stress  — 重複可靠性
 */

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/devicetree.h>
#include <string.h>

/* ── 測試區域 ────────────────────────────────────────────────────────────── */
#define SECTOR_SIZE     4096u
#define PAGE_SIZE       256u
#define TEST_SECTORS    4u                          /* 使用前 4 個 sector */
#define TEST_SIZE       (SECTOR_SIZE * TEST_SECTORS)/* 16 KB */

static const struct device *flash_dev;
static off_t                base;   /* storage_partition 起始實體位址 */
static size_t               part_size;

static uint8_t wbuf[TEST_SIZE];
static uint8_t rbuf[TEST_SIZE];

/* ── 工具函式 ────────────────────────────────────────────────────────────── */
static void erase_test_area(size_t sectors)
{
	int rc = flash_erase(flash_dev, base, SECTOR_SIZE * sectors);
	zassert_ok(rc, "erase %zu sectors failed: %d", sectors, rc);
}

static void verify_ff(off_t off, size_t len)
{
	int rc = flash_read(flash_dev, base + off, rbuf, len);
	zassert_ok(rc, "read failed: %d", rc);
	for (size_t i = 0; i < len; i++) {
		zassert_equal(rbuf[i], 0xFF,
			"[0x%lx+%zu] = 0x%02x, expected 0xFF",
			(unsigned long)off, i, rbuf[i]);
	}
}

/* ── Suite setup ─────────────────────────────────────────────────────────── */
static void *suite_setup(void)
{
	flash_dev  = PARTITION_DEVICE(storage_partition);
	base       = PARTITION_OFFSET(storage_partition);
	part_size  = PARTITION_SIZE(storage_partition);

	zassert_not_null(flash_dev, "flash device NULL");
	zassert_true(device_is_ready(flash_dev), "flash not ready");
	zassert_true(part_size >= TEST_SIZE,
		"partition too small: %zu < %u", part_size, TEST_SIZE);

	printk("[flash_test] dev=%s base=0x%lx size=%zu\n",
	       flash_dev->name, (unsigned long)base, part_size);
	return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Suite: basic
 * ═══════════════════════════════════════════════════════════════════════════ */
ZTEST_SUITE(suite_basic, NULL, suite_setup, NULL, NULL, NULL);

/* 擦除後全 0xFF */
ZTEST(suite_basic, test_erase_ff)
{
	erase_test_area(1);
	verify_ff(0, SECTOR_SIZE);
}

/* 寫遞增 pattern，讀回比對 */
ZTEST(suite_basic, test_write_read_incremental)
{
	erase_test_area(1);
	for (size_t i = 0; i < SECTOR_SIZE; i++) {
		wbuf[i] = (uint8_t)(i & 0xFF);
	}
	zassert_ok(flash_write(flash_dev, base, wbuf, SECTOR_SIZE), "write");
	zassert_ok(flash_read(flash_dev, base, rbuf, SECTOR_SIZE), "read");
	zassert_mem_equal(rbuf, wbuf, SECTOR_SIZE, "incremental mismatch");
}

/* 多 sector 擦除 */
ZTEST(suite_basic, test_multi_sector_erase)
{
	/* 先各 sector 寫不同值 */
	for (size_t s = 0; s < TEST_SECTORS; s++) {
		erase_test_area(1);
		memset(wbuf, (uint8_t)(0x10 + s), SECTOR_SIZE);
		zassert_ok(flash_write(flash_dev, base + s * SECTOR_SIZE,
				wbuf, SECTOR_SIZE), "write s%zu", s);
	}
	/* 一次擦除全部 */
	zassert_ok(flash_erase(flash_dev, base, TEST_SIZE), "multi-erase");
	verify_ff(0, TEST_SIZE);
}

/* 單 byte 讀寫 */
ZTEST(suite_basic, test_single_byte)
{
	uint8_t val = 0xA5;
	uint8_t out = 0x00;

	erase_test_area(1);
	zassert_ok(flash_write(flash_dev, base, &val, 1), "write 1B");
	zassert_ok(flash_read(flash_dev, base, &out, 1), "read 1B");
	zassert_equal(out, val, "got 0x%02x", out);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Suite: pattern
 * ═══════════════════════════════════════════════════════════════════════════ */
ZTEST_SUITE(suite_pattern, NULL, suite_setup, NULL, NULL, NULL);

/* 全 0x00 */
ZTEST(suite_pattern, test_all_zeros)
{
	erase_test_area(1);
	memset(wbuf, 0x00, SECTOR_SIZE);
	zassert_ok(flash_write(flash_dev, base, wbuf, SECTOR_SIZE), "write");
	zassert_ok(flash_read(flash_dev, base, rbuf, SECTOR_SIZE), "read");
	zassert_mem_equal(rbuf, wbuf, SECTOR_SIZE, "all-zero mismatch");
}

/* 交替 0xAA / 0x55 */
ZTEST(suite_pattern, test_alternating_aa55)
{
	erase_test_area(1);
	for (size_t i = 0; i < SECTOR_SIZE; i++) {
		wbuf[i] = (i & 1) ? 0x55 : 0xAA;
	}
	zassert_ok(flash_write(flash_dev, base, wbuf, SECTOR_SIZE), "write");
	zassert_ok(flash_read(flash_dev, base, rbuf, SECTOR_SIZE), "read");
	zassert_mem_equal(rbuf, wbuf, SECTOR_SIZE, "AA/55 mismatch");
}

/* Walking ones（每個 byte 只有一個 bit 為 1）*/
ZTEST(suite_pattern, test_walking_ones)
{
	erase_test_area(1);
	for (size_t i = 0; i < SECTOR_SIZE; i++) {
		wbuf[i] = (uint8_t)(1u << (i % 8));
	}
	zassert_ok(flash_write(flash_dev, base, wbuf, SECTOR_SIZE), "write");
	zassert_ok(flash_read(flash_dev, base, rbuf, SECTOR_SIZE), "read");
	zassert_mem_equal(rbuf, wbuf, SECTOR_SIZE, "walking-ones mismatch");
}

/* Walking zeros */
ZTEST(suite_pattern, test_walking_zeros)
{
	erase_test_area(1);
	for (size_t i = 0; i < SECTOR_SIZE; i++) {
		wbuf[i] = (uint8_t)(~(1u << (i % 8)));
	}
	zassert_ok(flash_write(flash_dev, base, wbuf, SECTOR_SIZE), "write");
	zassert_ok(flash_read(flash_dev, base, rbuf, SECTOR_SIZE), "read");
	zassert_mem_equal(rbuf, wbuf, SECTOR_SIZE, "walking-zeros mismatch");
}

/* 偽隨機（LFSR 16-bit）*/
ZTEST(suite_pattern, test_pseudo_random)
{
	uint16_t lfsr = 0xACE1u;

	erase_test_area(1);
	for (size_t i = 0; i < SECTOR_SIZE; i++) {
		wbuf[i] = (uint8_t)(lfsr & 0xFF);
		lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
	}
	zassert_ok(flash_write(flash_dev, base, wbuf, SECTOR_SIZE), "write");
	zassert_ok(flash_read(flash_dev, base, rbuf, SECTOR_SIZE), "read");
	zassert_mem_equal(rbuf, wbuf, SECTOR_SIZE, "LFSR mismatch");
}

/* 各 sector 寫不同 pattern，驗證互不干擾 */
ZTEST(suite_pattern, test_sector_isolation)
{
	erase_test_area(TEST_SECTORS);
	for (size_t s = 0; s < TEST_SECTORS; s++) {
		memset(wbuf, (uint8_t)(0x30 + s * 0x11), SECTOR_SIZE);
		zassert_ok(flash_write(flash_dev,
				base + s * SECTOR_SIZE, wbuf, SECTOR_SIZE),
			"write s%zu", s);
	}
	for (size_t s = 0; s < TEST_SECTORS; s++) {
		uint8_t expected = (uint8_t)(0x30 + s * 0x11);

		zassert_ok(flash_read(flash_dev,
				base + s * SECTOR_SIZE, rbuf, SECTOR_SIZE),
			"read s%zu", s);
		for (size_t i = 0; i < SECTOR_SIZE; i++) {
			zassert_equal(rbuf[i], expected,
				"s%zu[%zu]=0x%02x", s, i, rbuf[i]);
		}
	}
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Suite: boundary
 * ═══════════════════════════════════════════════════════════════════════════ */
ZTEST_SUITE(suite_boundary, NULL, suite_setup, NULL, NULL, NULL);

/* 跨 page boundary（256 B）*/
ZTEST(suite_boundary, test_cross_page)
{
	const size_t len = PAGE_SIZE;
	const off_t  off = PAGE_SIZE - (PAGE_SIZE / 2); /* 128 B into first page */

	erase_test_area(1);
	for (size_t i = 0; i < len; i++) {
		wbuf[i] = (uint8_t)(0xB0 + (i & 0x0F));
	}
	zassert_ok(flash_write(flash_dev, base + off, wbuf, len), "write");
	zassert_ok(flash_read(flash_dev, base + off, rbuf, len), "read");
	zassert_mem_equal(rbuf, wbuf, len, "cross-page mismatch");
}

/* 跨 sector boundary（4 KB）*/
ZTEST(suite_boundary, test_cross_sector)
{
	const size_t len = PAGE_SIZE * 2;
	const off_t  off = SECTOR_SIZE - PAGE_SIZE; /* last page of sector 0 */

	erase_test_area(2);
	for (size_t i = 0; i < len; i++) {
		wbuf[i] = (uint8_t)(0xC0 ^ i);
	}
	zassert_ok(flash_write(flash_dev, base + off, wbuf, len), "write");
	zassert_ok(flash_read(flash_dev, base + off, rbuf, len), "read");
	zassert_mem_equal(rbuf, wbuf, len, "cross-sector mismatch");
}

/* 最後一個 byte 寫入 */
ZTEST(suite_boundary, test_last_byte_of_sector)
{
	uint8_t val = 0xDE;
	uint8_t out = 0x00;
	const off_t off = SECTOR_SIZE - 1;

	erase_test_area(1);
	zassert_ok(flash_write(flash_dev, base + off, &val, 1), "write");
	zassert_ok(flash_read(flash_dev, base + off, &out, 1), "read");
	zassert_equal(out, val, "got 0x%02x", out);
}

/* 非對齊 offset（奇數位址）*/
ZTEST(suite_boundary, test_unaligned_offset)
{
	const size_t len = 17;
	const off_t  off = 3;  /* 非 4-byte 對齊 */

	erase_test_area(1);
	for (size_t i = 0; i < len; i++) {
		wbuf[i] = (uint8_t)(0x70 + i);
	}
	zassert_ok(flash_write(flash_dev, base + off, wbuf, len), "write");
	zassert_ok(flash_read(flash_dev, base + off, rbuf, len), "read");
	zassert_mem_equal(rbuf, wbuf, len, "unaligned mismatch");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Suite: error
 * ═══════════════════════════════════════════════════════════════════════════ */
ZTEST_SUITE(suite_error, NULL, suite_setup, NULL, NULL, NULL);

/* 寫入超出 flash 範圍 → -EINVAL */
ZTEST(suite_error, test_write_out_of_bounds)
{
	size_t flash_size = DT_REG_SIZE(DT_NODELABEL(flash0));
	uint8_t buf[1] = {0xAB};

	int rc = flash_write(flash_dev, (off_t)flash_size, buf, 1);
	zassert_not_equal(rc, 0, "expected error for out-of-bounds write");
}

/* 讀取超出 flash 範圍 → -EINVAL */
ZTEST(suite_error, test_read_out_of_bounds)
{
	size_t flash_size = DT_REG_SIZE(DT_NODELABEL(flash0));
	uint8_t buf[1];

	int rc = flash_read(flash_dev, (off_t)flash_size, buf, 1);
	zassert_not_equal(rc, 0, "expected error for out-of-bounds read");
}

/* 長度為 0 → OK（no-op）*/
ZTEST(suite_error, test_zero_length_ops)
{
	zassert_ok(flash_read(flash_dev, base, rbuf, 0), "read len=0");
	zassert_ok(flash_write(flash_dev, base, wbuf, 0), "write len=0");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Suite: stress
 * ═══════════════════════════════════════════════════════════════════════════ */
ZTEST_SUITE(suite_stress, NULL, suite_setup, NULL, NULL, NULL);

/* 反覆擦寫同一 sector 10 次 */
ZTEST(suite_stress, test_repeated_erase_write)
{
#define STRESS_CYCLES 10
	for (int c = 0; c < STRESS_CYCLES; c++) {
		uint8_t fill = (uint8_t)(0x80 + c);

		zassert_ok(flash_erase(flash_dev, base, SECTOR_SIZE),
			"erase cycle %d", c);
		memset(wbuf, fill, SECTOR_SIZE);
		zassert_ok(flash_write(flash_dev, base, wbuf, SECTOR_SIZE),
			"write cycle %d", c);
		zassert_ok(flash_read(flash_dev, base, rbuf, SECTOR_SIZE),
			"read cycle %d", c);
		for (size_t i = 0; i < SECTOR_SIZE; i++) {
			zassert_equal(rbuf[i], fill,
				"cycle %d byte[%zu]=0x%02x", c, i, rbuf[i]);
		}
	}
}

/* 4 個 sector 連續寫入後逐一驗證，再全部擦除驗證 */
ZTEST(suite_stress, test_sequential_sectors)
{
	erase_test_area(TEST_SECTORS);

	for (size_t s = 0; s < TEST_SECTORS; s++) {
		uint8_t fill = (uint8_t)(0xF0 - s * 0x10);

		memset(wbuf, fill, SECTOR_SIZE);
		zassert_ok(flash_write(flash_dev,
				base + s * SECTOR_SIZE, wbuf, SECTOR_SIZE),
			"write s%zu", s);
	}

	for (size_t s = 0; s < TEST_SECTORS; s++) {
		uint8_t expected = (uint8_t)(0xF0 - s * 0x10);

		zassert_ok(flash_read(flash_dev,
				base + s * SECTOR_SIZE, rbuf, SECTOR_SIZE),
			"read s%zu", s);
		for (size_t i = 0; i < SECTOR_SIZE; i++) {
			zassert_equal(rbuf[i], expected,
				"s%zu[%zu]=0x%02x", s, i, rbuf[i]);
		}
	}

	zassert_ok(flash_erase(flash_dev, base, TEST_SIZE), "final erase");
	verify_ff(0, TEST_SIZE);
}
