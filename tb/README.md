# Verification & Testbench Plan

Thư mục này chứa môi trường mô phỏng, testbench, mô hình kiểm chứng (golden reference model) và các vector kiểm thử cho **WOTS+ Accelerator**.

---

## 1. Cấu trúc Kiểm chứng (Verification Hierarchy)

1. **Unit Testbenches:**
   * `tb_base_w_checksum.sv`: Kiểm tra chuyển đổi 16-byte message sang 35 giá trị base-16 ($a[0..31]$) và 3 giá trị checksum ($a[32..34]$) với các trường hợp biên: all-zero, all-`0xFF`, mẫu luân phiên và ngẫu nhiên.
   * `tb_adrs_manager.sv`: Kiểm tra cập nhật địa chỉ `chain_address`, `hash_address` và chuyển đổi giữa `WOTS_HASH` và `WOTS_PK`.
   * `tb_chain_lane.sv`: Kiểm tra 1 lane thực thi chuỗi $F$ từ 0 bước đến 15 bước.
   * `tb_reorder_buffer.sv`: Kiểm tra gom và đảo thứ tự 35 chain khi các lane hoàn thành không đồng thời.
   * `tb_tl_stream_controller.sv`: Kiểm tra nén $T_\ell$ với chuỗi 560 byte trải dài nhiều block SHAKE256.

2. **Backend Integration Testbench:**
   * `tb_hash_adapter.sv`: Kiểm tra ghép nối giữa adapter định dạng FIPS 205 và IP SHAKE256 thực tế của nhóm nghiên cứu.

3. **Top-Level SoC / DMA Testbench:**
   * `tb_wots_accel_top.sv`: Mô phỏng đầy đủ với mô hình RAM ngoài (External RAM behavioral model) và bộ điều khiển DMA nạp descriptor.
   * Kiểm tra cả 3 chế độ:
     * `WOTS_PKGEN`: Khớp 100% với vector khóa công khai của phần mềm tham chiếu.
     * `WOTS_SIGN` $\rightarrow$ `WOTS_PK_FROM_SIG`: Tạo chữ ký rồi khôi phục lại đúng khóa công khai ban đầu.
     * Fault injection: Sửa 1 byte của message hoặc signature phải cho ra khóa công khai sai.

---

## 2. Nguồn Vector Mẫu (Golden Test Vectors)

- Dùng phần mềm tham chiếu FIPS 205 độc lập ([`slhdsa-c`](https://github.com/pq-code-package/slhdsa-c) hoặc kho C nội bộ tại `../../Post_Quantum_Cryptography/software/sphincsplus-dev/ref/`) để xuất các giá trị trung gian (intermediate values) theo từng bước chain, từng index và kiểm tra đối chiếu tự động.
