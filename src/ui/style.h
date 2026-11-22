#ifndef STYLE_H
#define STYLE_H

/**
 * @file style.h
 * @brief Token gaya semantik dan pemetaannya ke ANSI.
 *
 * Modul tidak menulis escape ANSI sendiri. Pilih token yang sesuai makna
 * (bahaya, sukses, rarity, HP), lalu render/ui yang memilih warna terbaik
 * sesuai kemampuan terminal (256 warna, 16 warna, atau mono).
 */

/**
 * @brief Token gaya; pilih berdasarkan makna, bukan warna.
 */
typedef enum {
    ST_TEXT = 0,        /**< teks biasa */
    ST_TITLE,           /**< judul layar: tebal + aksen */
    ST_ACCENT,          /**< aksen: marker, judul panel, angka penting */
    ST_BORDER,          /**< garis dan sudut panel */
    ST_DIM,             /**< teks sekunder/hint */
    ST_GOLD,            /**< nilai emas/ekonomi */
    ST_DANGER,          /**< bahaya, gagal, HP rendah */
    ST_SUCCESS,         /**< berhasil, pulih, aman */
    ST_RARITY_COMMON,   /**< item rarity 1 */
    ST_RARITY_RARE,     /**< item rarity 2 */
    ST_RARITY_EPIC,     /**< item rarity 3 */
    ST_RARITY_LEGENDARY,/**< item rarity 4 */
    ST_BRONZE,          /**< aksen sekunder (mis. peringkat) */
    ST_SELECTED,        /**< latar baris terpilih; warna depan dipilih widget */
    ST_FOG1,            /**< kabut jarak dekat */
    ST_FOG2,            /**< kabut jarak sedang */
    ST_FOG3,            /**< kabut jarak jauh */
    ST_BIOME_BASE,      /**< dasar gaya dinding biome; jangan dipakai langsung */
    ST_BIOME_FOG_BASE = ST_BIOME_BASE + 8, /**< dasar gaya dinding berkabut */
    ST_HP_GRAD_BASE = ST_BIOME_FOG_BASE + 24, /**< dasar gradasi bar HP */
    ST_COUNT
} Style;

/**
 * @brief Token gradasi bar HP pada slot ke-i.
 * @param i indeks slot 0..10; naik seiring HP.
 */
#define ST_HP_GRAD(i) ((Style)(ST_HP_GRAD_BASE + (i)))
/**
 * @brief Token gaya dinding biome ke-i.
 * @param i indeks biome 0..7.
 */
#define ST_BIOME(i) ((Style)(ST_BIOME_BASE + (i)))
/**
 * @brief Token gaya kabut pada tingkat ke-i.
 * @param i 0 = dekat, 1 = sedang, 2 = jauh.
 */
#define ST_FOG_LEVEL(i) ((Style)(ST_FOG1 + (i)))
/**
 * @brief Token gaya dinding biome b pada tingkat kabut tier.
 * @param tier 0..BIOME_FOG_TIERS-1;
 * @param bi indeks biome 0..BIOME_COUNT-1.
 */
#define ST_BIOME_FOG(tier, bi) ((Style)(ST_BIOME_FOG_BASE + (tier) * BIOME_COUNT + (bi)))
/** @brief Jumlah biome yang punya gaya dinding. */
#define BIOME_COUNT 8
/** @brief Jumlah tingkat kabut untuk dinding biome. */
#define BIOME_FOG_TIERS 3

/**
 * @brief Bangun tabel ANSI sesuai kemampuan terminal yang terdeteksi.
 * @note Dipanggil ui_init(); bukan untuk modul.
 */
void style_init(void);

/**
 * @brief Token gaya untuk tingkat rarity item.
 * @param rarity 1..4 (di luar itu jatuh ke common).
 * @return ST_RARITY_COMMON..ST_RARITY_LEGENDARY.
 */
Style style_rarity(int rarity);

/**
 * @brief Token gaya untuk HP berdasarkan ambang (aman/sedang/bahaya).
 * @param hp nilai HP sekarang;
 * @param maxHp nilai maksimum.
 * @return token gaya yang sesuai.
 */
Style style_hp(int hp, int maxHp);

/**
 * @brief Urutan escape warna depan untuk satu token.
 * @param s token gaya.
 * @return string ANSI, atau "" pada mode mono.
 */
const char *style_fg(Style s);

/**
 * @brief Urutan escape warna latar untuk satu token.
 * @param s token gaya.
 * @return string ANSI, atau "" pada mode mono.
 */
const char *style_bg(Style s);

#endif
