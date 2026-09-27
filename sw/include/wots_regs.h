/**
 * @file wots_regs.h
 * @brief Register Map and Hardware Interface for WOTS+ Hardware Accelerator
 * @details Target algorithm: NIST FIPS 205 SLH-DSA-SHAKE-128f
 */

#ifndef WOTS_REGS_H
#define WOTS_REGS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* WOTS+ Algorithm Parameters (SLH-DSA-SHAKE-128f)                           */
/* ========================================================================== */
#define WOTS_PARAM_N             16   /* Length of hash output and seeds (bytes) */
#define WOTS_PARAM_W             16   /* Winternitz parameter */
#define WOTS_PARAM_LOG_W         4    /* log2(w) */
#define WOTS_PARAM_LEN1          32   /* ceil(8*N / log2(w)) */
#define WOTS_PARAM_LEN2          3    /* ceil(floor(log2(LEN1 * (w-1))) / log2(w)) + 1 */
#define WOTS_PARAM_LEN           35   /* Total chain count: LEN1 + LEN2 */
#define WOTS_VECTOR_BYTES        560  /* 35 chains * 16 bytes = 560 bytes */
#define WOTS_PK_BYTES            16   /* Compressed WOTS+ public key */
#define WOTS_ADRS_BYTES          32   /* Address size in bytes */

/* ========================================================================== */
/* CSR Register Offsets                                                       */
/* ========================================================================== */
#define WOTS_REG_CONTROL         0x00 /* Control register (R/W) */
#define WOTS_REG_STATUS          0x04 /* Status register (RO) */
#define WOTS_REG_DESC_BASE       0x08 /* Descriptor base address in external RAM (R/W) */
#define WOTS_REG_JOB_COUNT       0x0C /* Number of WOTS operations in batch (R/W) */
#define WOTS_REG_SRC_BASE        0x10 /* Source buffer base address in external RAM (R/W) */
#define WOTS_REG_DST_BASE        0x14 /* Destination buffer base address in external RAM (R/W) */
#define WOTS_REG_IRQ_ENABLE      0x18 /* Interrupt enable register (R/W) */
#define WOTS_REG_IRQ_STATUS      0x1C /* Interrupt status register (R/W1C - Write 1 to Clear) */

/* Performance Counters */
#define WOTS_REG_PERF_CYCLES     0x20 /* Total elapsed execution clock cycles */
#define WOTS_REG_PERF_LANE0_BUSY 0x24 /* Active compute cycles for Lane 0 */
#define WOTS_REG_PERF_LANE1_BUSY 0x28 /* Active compute cycles for Lane 1 */
#define WOTS_REG_PERF_DMA_STALL  0x2C /* Cycles stalled waiting for DMA burst transfers */
#define WOTS_REG_PERF_HASH_STALL 0x30 /* Cycles stalled waiting for hash backend adapter */
#define WOTS_REG_PERF_REORDER    0x34 /* Cycles spent in reorder buffer synchronization */

/* ========================================================================== */
/* Control Register Bit Masks (WOTS_REG_CONTROL)                             */
/* ========================================================================== */
#define WOTS_CTRL_START_MASK     (1u << 0) /* 1: Start accelerator execution */
#define WOTS_CTRL_RESET_MASK     (1u << 1) /* 1: Soft reset accelerator logic */
#define WOTS_CTRL_MODE_SHIFT     2
#define WOTS_CTRL_MODE_MASK      (0x3u << WOTS_CTRL_MODE_SHIFT)

/* Operation Modes */
#define WOTS_MODE_PKGEN          (0x0u << WOTS_CTRL_MODE_SHIFT) /* wots_pkGen: generates PK from seeds */
#define WOTS_MODE_SIGN           (0x1u << WOTS_CTRL_MODE_SHIFT) /* wots_sign: signs a 16-byte message */
#define WOTS_MODE_PK_FROM_SIG    (0x2u << WOTS_CTRL_MODE_SHIFT) /* wots_pkFromSig: verifies & recovers PK */

/* ========================================================================== */
/* Status Register Bit Masks (WOTS_REG_STATUS)                               */
/* ========================================================================== */
#define WOTS_STATUS_BUSY_MASK    (1u << 0) /* 1: Accelerator is currently active */
#define WOTS_STATUS_DONE_MASK    (1u << 1) /* 1: Operation completed successfully */
#define WOTS_STATUS_ERROR_MASK   (1u << 2) /* 1: Error encountered during execution */
#define WOTS_STATUS_ERRCODE_SHIFT 3
#define WOTS_STATUS_ERRCODE_MASK (0x7u << WOTS_STATUS_ERRCODE_SHIFT)

/* Error Codes */
#define WOTS_ERR_NONE            0x0
#define WOTS_ERR_DMA_RESP        0x1 /* Bus error or DMA response timeout */
#define WOTS_ERR_INVALID_PARAM   0x2 /* Invalid descriptor or parameter */
#define WOTS_ERR_HASH_TIMEOUT    0x3 /* Hash backend failed to respond */

/* ========================================================================== */
/* Interrupt Register Bit Masks (IRQ_ENABLE / IRQ_STATUS)                    */
/* ========================================================================== */
#define WOTS_IRQ_DONE_MASK       (1u << 0) /* Interrupt on batch completion */
#define WOTS_IRQ_ERROR_MASK      (1u << 1) /* Interrupt on execution error */

/* ========================================================================== */
/* DMA Descriptor Format (Aligned to 32 bytes in External RAM)                */
/* ========================================================================== */
typedef struct __attribute__((packed, aligned(32))) {
    uint32_t op_mode;          /* Operation mode: PKGEN, SIGN, PK_FROM_SIG */
    uint32_t flags;            /* Control flags (interrupt on completion, etc.) */
    uint32_t src_addr;         /* Physical address of input data in RAM */
    uint32_t dst_addr;         /* Physical address of output buffer in RAM */
    uint32_t adrs_addr;        /* Physical address of 32-byte ADRS in RAM */
    uint32_t job_id;           /* Software-assigned tracking ID */
    uint32_t reserved[2];      /* Reserved for future extensions / padding */
} wots_dma_desc_t;

#ifdef __cplusplus
}
#endif

#endif /* WOTS_REGS_H */
