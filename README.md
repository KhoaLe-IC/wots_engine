# Multi-Lane WOTS+ Hardware Accelerator for FIPS 205 SLH-DSA-SHAKE-128f

Thiết kế bộ tăng tốc phần cứng (Hardware Accelerator IP Core) đa lane cho thuật toán chữ ký một lần **WOTS+ (Winternitz One-Time Signature)** tuân thủ tiêu chuẩn mật mã hậu lượng tử **NIST FIPS 205 (SLH-DSA)**, tích hợp trong hệ thống SoC sử dụng DMA và bộ nhớ RAM ngoài.

---

## 📌 Tổng quan Dự án (Project Overview)

- **Thuật toán mục tiêu:** NIST FIPS 205 — `SLH-DSA-SHAKE-128f`
- **Bộ tham số khóa:** $n = 16$ bytes, $w = 16$ ($\log_2(w) = 4$), $len_1 = 32$, $len_2 = 3$, $len = 35$ chuỗi Winternitz độc lập.
- **Vai trò trong SLH-DSA:** WOTS+ là thành phần cốt lõi xuất hiện trong cả 3 luồng:
  1. `wots_pkGen`: Tạo khóa công khai WOTS+ để hình thành các lá XMSS và root hypertree (KeyGen).
  2. `wots_sign`: Ký digest thông điệp/gốc cây tại từng tầng hypertree (Sign).
  3. `wots_pkFromSig`: Khôi phục lại khóa công khai từ chữ ký để xác thực đường đi (Verify).
- **Đặc trưng kiến trúc:**
  - **Đa lane phần cứng (Multi-lane):** Tối thiểu 2 lane thực thi chuỗi song song thực thụ.
  - **Lập lịch động (Dynamic Scheduling):** Phân phối 35 chuỗi có độ dài khác nhau cho các lane rảnh.
  - **Bộ đệm hoàn thiện (Reorder Buffer):** Sắp xếp kết quả các chuỗi hoàn thành không đồng thời về đúng thứ tự $0 \to 34$.
  - **Nén phần cứng $T_\ell$:** Tự động hấp thụ 560 byte ($35 \times 16$) để sinh ra khóa công khai 16 byte.
  - **Tích hợp SoC qua DMA:** Tự động đọc descriptor/input và ghi kết quả ra RAM ngoài không làm nghẽn bus CPU.

---

## 📁 Cấu trúc Thư mục (Repository Layout)

```text
WOTS_Engine/
├── README.md                      # Giới thiệu tổng quan và hướng dẫn dự án
├── .gitignore                     # Bỏ qua tệp build EDA (Vivado, Quartus, Questa), waveform, tệp tạm
│
├── docs/                          # Tài liệu đặc tả kỹ thuật & kế hoạch
│   ├── KE_HOACH_CHINH_THUC_SOC_FIPS205.md   # Kế hoạch chính thức đồ án SoC & bài báo FIPS 205
│   └── WOTS_ACCELERATOR_SPEC.md             # Đặc tả kỹ thuật phần cứng chi tiết v0.1 (Architecture & ABI)
│
├── rtl/                           # Mã nguồn phần cứng (SystemVerilog / Verilog)
│   └── README.md                  # Bản phân rã 13 module RTL và quy ước thiết kế
│
├── tb/                            # Môi trường mô phỏng & kiểm chứng (Testbench)
│   └── README.md                  # Kế hoạch kiểm thử Unit-test, Top-level SoC và Golden Vectors
│
└── sw/                            # Phần mềm điều khiển (Driver / Baremetal firmware)
    ├── README.md                  # Hướng dẫn tích hợp phần mềm và luồng DMA
    └── include/
        └── wots_regs.h            # Tệp header C định nghĩa Register Map và DMA Descriptor
```

---

## 🚀 3 Chế độ Hoạt động Phần cứng

| Chế độ | Chuẩn FIPS 205 | Dữ liệu đầu vào | Dữ liệu đầu ra |
|---|---|---|---|
| **`WOTS_PKGEN`** | Algorithm 6 (`wots_pkGen`) | `SK.seed`, `PK.seed`, `ADRS` | WOTS+ Public Key (16 bytes sau khi nén $T_\ell$) |
| **`WOTS_SIGN`** | Algorithm 7 (`wots_sign`) | `SK.seed`, `PK.seed`, `ADRS`, Message (16B) | WOTS+ Signature Vector (560 bytes) |
| **`WOTS_PK_FROM_SIG`** | Algorithm 8 (`wots_pkFromSig`) | `PK.seed`, `ADRS`, Message (16B), Sig (560B) | Candidate WOTS+ Public Key (16 bytes) |

---

## 🛠️ Giao diện & Bản đồ Thanh ghi (CSR Map)

Accelerator được điều khiển qua tập thanh ghi CSR 32-bit (xem chi tiết tại [`sw/include/wots_regs.h`](sw/include/wots_regs.h)):

| Offset | Tên thanh ghi | R/W | Chức năng |
|:---:|:---|:---:|:---|
| `0x00` | `WOTS_REG_CONTROL` | R/W | `[0]` Start, `[1]` Soft Reset, `[3:2]` Mode (PKGEN / SIGN / PK_FROM_SIG) |
| `0x04` | `WOTS_REG_STATUS` | RO | `[0]` Busy, `[1]` Done, `[2]` Error, `[5:3]` Error Code |
| `0x08` | `WOTS_REG_DESC_BASE` | R/W | Địa chỉ cơ sở của mảng Descriptor trong RAM ngoài |
| `0x0C` | `WOTS_REG_JOB_COUNT` | R/W | Số lượng công việc WOTS cần xử lý trong batch |
| `0x18` | `WOTS_REG_IRQ_ENABLE` | R/W | Bật ngắt hoàn thành tác vụ (`DONE`) hoặc lỗi (`ERROR`) |
| `0x1C` | `WOTS_REG_IRQ_STATUS` | W1C | Trạng thái ngắt hiện tại và xóa cờ ngắt |
| `0x20 - 0x34` | `WOTS_REG_PERF_*` | RO | Các bộ đếm chu kỳ hiệu năng (tổng chu kỳ, bận lane, stall DMA/Hash) |

---

## 📖 Tài liệu Tham khảo & Bản quyền

- [Kế hoạch chính thức đồ án SoC](docs/KE_HOACH_CHINH_THUC_SOC_FIPS205.md)
- [Đặc tả kỹ thuật WOTS+ Accelerator v0.1](docs/WOTS_ACCELERATOR_SPEC.md)
- **Nguồn Backend SHAKE256:** Tái sử dụng IP lõi SHAKE256/Keccak của nhóm nghiên cứu PQC.
- **Chuẩn tham chiếu:** [NIST FIPS 205 (SLH-DSA)](https://doi.org/10.6028/NIST.FIPS.205) và [NIST FIPS 202 (SHA-3 & SHAKE)](https://doi.org/10.6028/NIST.FIPS.202).
