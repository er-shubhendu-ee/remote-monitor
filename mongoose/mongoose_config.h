
#pragma once

/*
 * Custom Mongoose configuration for ESP32
 * Based on upstream defaults (src/config.h),
 * extended with application-specific overrides and
 * common example options.
 *
 * Notes:
 * - Upstream config.h uses #ifndef guards, so your
 *   definitions here override defaults automatically.
 * - Keep this file included before <mongoose.h>.
 */

/* ============================================================
 * Architecture selection
 * ============================================================
 */
#define MG_ARCH MG_ARCH_ESP32  // Target: ESP32 platform

/* ============================================================
 * TLS / Security
 * ============================================================
 */
#define MG_ENABLE_MBEDTLS 0   // Disable mbedTLS wrapper (ESP-IDF provides its own)
#define MG_ENABLE_OPENSSL 0   // Disable OpenSSL wrapper
#define MG_TLS MG_TLS_NONE    // Options: MG_TLS_NONE, MG_TLS_BUILTIN, MG_TLS_MBED, MG_TLS_OPENSSL
#define MG_OTA MG_OTA_CUSTOM  // Use custom OTA implementation

/* ============================================================
 * System hooks
 * ============================================================
 */
#define MG_ENABLE_CUSTOM_RANDOM 0  // Use custom mg_random()? 0 = use Mongoose default
#define MG_ENABLE_CUSTOM_MILLIS 0  // Use custom mg_millis()? 0 = use Mongoose default

/* ============================================================
 * File system support
 * ============================================================
 */
#define MG_ENABLE_POSIX_FS 1                        // Enable POSIX fopen/fread/...
#define MG_ENABLE_PACKED_FS 0                       // Disable packed FS
#define MG_ENABLE_FATFS 0                           // Enable/Disable FAT FS
#define MG_FATFS_ROOT APP_CONFIG_STORAGE_ROOT_PATH  // Root path if FATFS is enabled

/* ============================================================
 * Logging
 * ============================================================
 */
// Show source file:line in logs (non-upstream extension)
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define MG_ENABLE_LOG 1
#else
#define MG_ENABLE_LOG 0
#endif

/* ============================================================
 * Buffers and limits
 * ============================================================
 */
#define MG_IO_SIZE (1UL * 8UL * 1024UL)        // Buffer growth granularity (default: 512)
#define MG_MAX_RECV_SIZE (8UL * 8UL * 1024UL)  // Max recv buffer size per conn (64 kB)
#define MG_MAX_HTTP_HEADERS 30                 // Max number of HTTP headers (default: 30)

/* ============================================================
 * HTTP server defaults
 * ============================================================
 */
#define MG_HTTP_INDEX "index.html"  // Default index file for directories

/* ============================================================
 * Example-inspired options
 * ============================================================
 * These appear in Mongoose example configs and may be useful.
 */
#define MG_ENABLE_HTTP 1               // Enable HTTP server core
#define MG_ENABLE_SSI 0                // Server-Side Includes (set 1 if needed)
#define MG_ENABLE_DIRECTORY_LISTING 1  // Enable HTTP directory listing
#define MG_ENABLE_HEXDUMP 0            // Disable connection hexdump by default
#define MG_ENABLE_MIP 0                // Disable Mongoose IP stack (use LwIP on ESP32)

/* ============================================================
 * End of configuration
 * ============================================================
 */
