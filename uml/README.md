# UML Quản lý thư viện

Sơ đồ lớp UML tương tác của chương trình quản lý thư viện (`test1.cpp`): 11 thẻ (9 lớp, 2 enum) và 21 quan hệ. Khung trang lấy từ mẫu `uml-ctdl-project2`, dữ liệu là các file JSON trong `data/`.

## Cách mở

Trình duyệt không cho trang mở bằng cách bấm đúp (`file://`) đọc các file JSON, nên cần một máy chủ cục bộ. Mở PowerShell và chạy:

```powershell
cd "D:\Dự án lập trình hướng đối tượng\uml"
py -m http.server 8000
```

Rồi mở `http://localhost:8000`. Thêm `#ThuVien` vào địa chỉ để mở thẳng một lớp. Nếu trang hiện bản cũ, nhấn Ctrl + F5.

## Cách xem

- Cuộn chuột để phóng to hoặc thu nhỏ, kéo để di chuyển. Có 4 mức chi tiết (tự đổi theo độ phóng).
- Số trên mỗi thẻ là thứ tự đọc; thanh bên liệt kê theo thứ tự đó.
- Rê chuột lên lớp, thành viên hoặc đường nối để xem chú thích; bấm để mở khung chi tiết bên phải. Tên lớp và tên kiểu đều bấm được.
- Mặc định chỉ vẽ quan hệ cấu trúc (kế thừa, sở hữu). Quan hệ phụ thuộc hiện khi chọn một lớp, hoặc bật «Hiện mọi quan hệ phụ thuộc» ở thanh bên.

## Các lớp

| Gói | Lớp |
| --- | --- |
| Tiện ích chung | `VanBan` (cùng thẻ «Kiểu cơ bản») |
| Tài liệu | `TaiLieu` (trừu tượng), `Sach`, `TapChi`, `Bao` |
| Nghiệp vụ thư viện | `ThuVien`, `TieuChiSapXep` (enum), `KetQua` (enum) |
| Giao diện và điều khiển | `GiaoDien`, `UngDung` |

## Các file

| File | Nội dung |
| --- | --- |
| `index.html`, `style.css`, `common.js`, `diagram.js`, `main.js` | Khung trang và mã vẽ (giữ nguyên từ mẫu; chỉ đổi tiêu đề, danh sách file dữ liệu và cách hiện hàm không có kiểu trả về) |
| `data/meta.json` | Tên, các gói (màu), các chặng đọc |
| `data/classes_types.json`, `classes_util.json`, `classes_model.json`, `classes_logic.json`, `classes_ui.json` | Nội dung từng lớp |
| `data/relations.json` | Các quan hệ giữa lớp |

Muốn sửa sơ đồ thì sửa các file JSON; không cần sửa mã trang. Khi thêm hoặc xóa lớp, cập nhật `read` (thứ tự đọc), `stage` (chặng) và `pos` (vị trí ban đầu) của lớp đó.
