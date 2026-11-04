#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cctype>
#include <conio.h>
#include <windows.h>

using namespace std;

// Cờ bật chế độ hiểu mã màu ANSI của console; định nghĩa lại phòng khi header cũ chưa có
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

// Tên file lưu dữ liệu thư viện
const string TEN_FILE = "thuvien.txt";

// Các mã màu ANSI; in mã màu trước chữ và in RESET sau chữ để trả về màu mặc định
const string RESET   = "\033[0m";
const string GRAY    = "\033[90m";
const string RED     = "\033[1;31m";
const string GREEN   = "\033[1;32m";
const string YELLOW  = "\033[1;33m";
const string BLUE    = "\033[1;34m";
const string MAGENTA = "\033[1;35m";
const string CYAN    = "\033[1;36m";
const string WHITE   = "\033[1;37m";
const string H_LIGHT = "\033[1;37;44m";  // chữ trắng nền xanh, tô sáng dòng đang chọn trong menu

// Lớp tiện ích chứa các hàm xử lý chuỗi, có tính đến UTF-8 tiếng Việt
class VanBan {
public:
    // Đếm số ký tự hiển thị của chuỗi UTF-8 (bỏ qua các byte nối tiếp của ký tự nhiều byte)
    static int doDaiHienThi(const string& s) {
        int count = 0;
        for (unsigned char c : s) {
            if ((c & 0xC0) != 0x80) count++;
        }
        return count;
    }

    // Thêm khoảng trắng cho đủ độ rộng width; căn trái mặc định, căn phải nếu alignRight = true
    static string canLe(const string& s, int width, bool alignRight = false) {
        int diff = width - doDaiHienThi(s);
        if (diff < 0) diff = 0;
        return alignRight ? (string(diff, ' ') + s) : (s + string(diff, ' '));
    }

    // Cắt chuỗi dài hơn maxLen ký tự và thêm ".." ở cuối
    static string catChuoi(const string& s, int maxLen) {
        if (doDaiHienThi(s) <= maxLen) return s;
        int soKyTu = 0, bytePos = 0;
        for (size_t i = 0; i < s.length(); ++i) {
            if ((s[i] & 0xC0) != 0x80) {
                soKyTu++;
                if (soKyTu > maxLen - 2) {
                    return s.substr(0, bytePos) + "..";
                }
            }
            bytePos = i + 1;
        }
        return s;
    }

    // Lặp chuỗi s đúng n lần, dùng để vẽ đường kẻ
    static string lapChuoi(const string& s, int n) {
        string res = "";
        for (int i = 0; i < n; ++i) res += s;
        return res;
    }

    // Đổi số tiền sang chuỗi có dấu chấm ngăn cách hàng nghìn, ví dụ 1234567 -> "1.234.567 đ"
    static string dinhDangTien(double tien) {
        long long val = (long long)(tien + 0.5);
        string s = to_string(val);
        int n = (int)s.length() - 3;
        while (n > 0) {
            s.insert(n, ".");
            n -= 3;
        }
        return s + " đ";
    }

    // Đổi chuỗi sang chữ thường, dùng để so sánh không phân biệt hoa thường
    static string chuThuong(string s) {
        for (char& c : s) c = tolower((unsigned char)c);
        return s;
    }

    // Bỏ khoảng trắng và ký tự xuống dòng ở hai đầu chuỗi
    static string catKhoangTrang(const string& s) {
        size_t first = s.find_first_not_of(" \t\r\n");
        if (first == string::npos) return "";
        size_t last = s.find_last_not_of(" \t\r\n");
        return s.substr(first, last - first + 1);
    }
};

// Lớp tiện ích lo mọi việc trên console: menu, kẻ bảng, nhập liệu, thông báo
class GiaoDien {
private:
    // Lấy số cột và số dòng của cửa sổ console; nếu lỗi thì dùng 120 x 30
    static void layKichThuocConsole(int& cols, int& rows) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        } else {
            cols = 120;
            rows = 30;
        }
    }

    // Căn chuỗi s vào giữa một khung rộng w bằng khoảng trắng hai bên
    static string canGiua(const string& s, int w) {
        int pad = w - VanBan::doDaiHienThi(s);
        int left = pad / 2, right = pad - left;
        return string(left, ' ') + s + string(right, ' ');
    }

public:
    // Chuẩn bị console: bảng mã UTF-8 và chế độ hiểu mã màu ANSI; gọi một lần khi chương trình bắt đầu
    static void khoiTao() {
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    // Xóa toàn bộ màn hình
    static void xoaManHinh() { system("cls"); }

    // Vẽ một đường kẻ ngang của bảng: viTri 0 là mép trên, 1 là đường giữa, 2 là mép dưới
    static void keKhungBang(int viTri) {
        string l = (viTri == 0) ? "┌" : (viTri == 1 ? "├" : "└");
        string m = (viTri == 0) ? "┬" : (viTri == 1 ? "┼" : "┴");
        string r = (viTri == 0) ? "┐" : (viTri == 1 ? "┤" : "┘");

        cout << GRAY << l
             << VanBan::lapChuoi("─", 10) << m
             << VanBan::lapChuoi("─", 26) << m
             << VanBan::lapChuoi("─", 17) << m
             << VanBan::lapChuoi("─", 8)  << m
             << VanBan::lapChuoi("─", 11) << m
             << VanBan::lapChuoi("─", 27) << m
             << VanBan::lapChuoi("─", 13) << m
             << VanBan::lapChuoi("─", 14) << r << RESET << "\n";
    }

    // In phần đầu của bảng tài liệu: mép trên, hàng tên cột và đường kẻ giữa
    static void inTieuDeBang() {
        keKhungBang(0);
        cout << GRAY << "│ " << YELLOW << VanBan::canLe("Mã TL", 8)
             << GRAY << " │ " << YELLOW << VanBan::canLe("Tên tài liệu", 24)
             << GRAY << " │ " << YELLOW << VanBan::canLe("Nhà xuất bản", 15)
             << GRAY << " │ " << YELLOW << VanBan::canLe("Năm", 6, true)
             << GRAY << " │ " << YELLOW << VanBan::canLe("Phân loại", 9)
             << GRAY << " │ " << YELLOW << VanBan::canLe("Thông tin đặc trưng", 25)
             << GRAY << " │ " << YELLOW << VanBan::canLe("Trạng thái", 11)
             << GRAY << " │ " << YELLOW << VanBan::canLe("Thành tiền", 12, true)
             << GRAY << " │" << RESET << "\n";
        keKhungBang(1);
    }

    // In khung tiêu đề của một chức năng, ví dụ "THÊM SÁCH MỚI"
    static void inTieuDeChucNang(const string& tieuDe) {
        int w = 58;
        cout << CYAN << "╔" << VanBan::lapChuoi("═", w) << "╗\n"
             << "║" << YELLOW << canGiua(tieuDe, w) << CYAN << "║\n"
             << "╚" << VanBan::lapChuoi("═", w) << "╝\n\n" << RESET;
    }

    // Hiện menu căn giữa màn hình, chọn bằng phím mũi tên lên/xuống và Enter.
    // Trả về chỉ số mục được chọn; nhấn Esc thì trả về mục cuối cùng.
    static int chonMenu(const string& tieuDe, const vector<string>& muc, const string& thongKe) {
        int chon = 0, n = (int)muc.size();
        int w = 58;
        xoaManHinh();
        cout << "\033[?25l";

        while (true) {
            int cols, rows;
            layKichThuocConsole(cols, rows);
            int chieuCaoMenu = n + 8;
            int leTren = max(1, (rows - chieuCaoMenu) / 2);
            int leTrai = max(0, (cols - (w + 2)) / 2);
            string padLeft(leTrai, ' ');

            cout << "\033[H" << string(leTren, '\n');
            cout << padLeft << CYAN << "╔" << VanBan::lapChuoi("═", w) << "╗\n";
            cout << padLeft << "║" << YELLOW << canGiua(tieuDe, w) << CYAN << "║\n";
            cout << padLeft << "╠" << VanBan::lapChuoi("═", w) << "╣\n";
            cout << padLeft << "║" << GREEN << canGiua(thongKe, w) << CYAN << "║\n";
            cout << padLeft << "╠" << VanBan::lapChuoi("─", w) << "╣\n" << RESET;

            for (int i = 0; i < n; ++i) {
                string dong = (i == chon ? "  ❯ " : "    ") + muc[i];
                cout << padLeft << CYAN << "║ " << RESET;
                if (i == chon) {
                    cout << H_LIGHT << VanBan::canLe(dong, w - 2) << RESET;
                } else {
                    cout << WHITE << VanBan::canLe(dong, w - 2) << RESET;
                }
                cout << CYAN << " ║\n" << RESET;
            }

            cout << padLeft << CYAN << "╠" << VanBan::lapChuoi("─", w) << "╣\n";
            cout << padLeft << "║" << GRAY << canGiua("[↑/↓] Di chuyển   [Enter] Chọn   [Esc] Thoát", w) << CYAN << "║\n";
            cout << padLeft << "╚" << VanBan::lapChuoi("═", w) << "╝\n" << RESET;

            int k = _getch();
            if (k == 224 || k == 0) {
                k = _getch();
                if (k == 72) chon = (chon - 1 + n) % n;
                else if (k == 80) chon = (chon + 1) % n;
            } else if (k == 13) {
                cout << "\033[?25h";
                return chon;
            } else if (k == 27) {
                cout << "\033[?25h";
                return n - 1;
            }
        }
    }

    // Chờ người dùng nhấn một phím bất kỳ; hanhDongTiep là câu cuối của lời nhắc
    static void tamDung(const string& hanhDongTiep = "quay về menu") {
        cout << GRAY << "\n  Nhấn phím bất kỳ để " << hanhDongTiep << "..." << RESET;
        _getch();
    }

    // Đọc một dòng từ bàn phím vào ketQua. Trả false nếu người dùng bỏ trống hoặc nhập 0 (hủy thao tác).
    static bool nhapCoThoat(const string& loiNhac, string& ketQua) {
        cout << CYAN << "  ▸ " << loiNhac << GRAY << " (Nhấn Enter hoặc nhập 0 để hủy): " << RESET;
        getline(cin, ketQua);
        ketQua = VanBan::catKhoangTrang(ketQua);
        if (ketQua.empty() || ketQua == "0") {
            cout << YELLOW << "  [i] Đã hủy thao tác.\n" << RESET;
            return false;
        }
        return true;
    }

    // Hỏi có/không bằng một phím; trả true nếu người dùng nhấn y hoặc Y
    static bool hoiXacNhan(const string& cauHoi) {
        cout << YELLOW << "\n  " << cauHoi << " (y/n): " << RESET;
        char c = _getch();
        cout << c << "\n";
        return c == 'y' || c == 'Y';
    }

    // In thông báo lỗi màu đỏ
    static void baoLoi(const string& noiDung)   { cout << RED    << "  [!] " << noiDung << "\n" << RESET; }

    // In thông báo thành công màu xanh lá
    static void baoThanhCong(const string& nd)  { cout << GREEN  << "  [✔] " << nd << "\n" << RESET; }

    // In thông báo thông tin màu vàng
    static void baoThongTin(const string& nd)   { cout << YELLOW << "  [i] " << nd << "\n" << RESET; }
};

// Lớp cơ sở trừu tượng cho mọi loại tài liệu; giữ dữ liệu chung và trạng thái mượn
class TaiLieu {
protected:
    string maTaiLieu;
    string tenTaiLieu;
    string nhaXuatBan;
    int namXuatBan;
    bool daMuon;

    // In các cột cuối của một dòng bảng (loại, thông tin đặc trưng, trạng thái, thành tiền);
    // các lớp con gọi hàm này trong Xuat để khỏi lặp lại cùng một đoạn in
    void xuatPhanDuoi(ostream& os, const string& loai, const string& mauLoai, const string& chiTiet) const {
        string tThai = daMuon ? "Đang mượn" : "Có sẵn";
        os << GRAY << " │ " << RESET << mauLoai << VanBan::canLe(loai, 9) << RESET
           << GRAY << " │ " << RESET << VanBan::canLe(VanBan::catChuoi(chiTiet, 25), 25)
           << GRAY << " │ " << RESET << (daMuon ? RED : GREEN) << VanBan::canLe(tThai, 11) << RESET
           << GRAY << " │ " << RESET << YELLOW << VanBan::canLe(VanBan::dinhDangTien(TinhTien()), 12, true) << RESET
           << GRAY << " │" << RESET;
    }

public:
    // Tạo tài liệu rỗng, dùng khi nhập dữ liệu từ bàn phím
    TaiLieu() : maTaiLieu(""), tenTaiLieu(""), nhaXuatBan(""), namXuatBan(2026), daMuon(false) {}

    // Tạo tài liệu với đầy đủ thông tin chung
    TaiLieu(string ma, string ten, string nxb, int nam, bool muon = false)
        : maTaiLieu(ma), tenTaiLieu(ten), nhaXuatBan(nxb), namXuatBan(nam), daMuon(muon) {}

    // Destructor ảo để xóa đúng đối tượng lớp con qua con trỏ lớp cha
    virtual ~TaiLieu() {}

    // Lấy mã tài liệu
    string getMaTaiLieu() const { return maTaiLieu; }
    // Đặt mã tài liệu
    void setMaTaiLieu(string ma) { maTaiLieu = ma; }

    // Lấy tên tài liệu
    string getTenTaiLieu() const { return tenTaiLieu; }
    // Đặt tên tài liệu
    void setTenTaiLieu(string ten) { tenTaiLieu = ten; }

    // Lấy nhà xuất bản
    string getNhaXuatBan() const { return nhaXuatBan; }
    // Đặt nhà xuất bản
    void setNhaXuatBan(string nxb) { nhaXuatBan = nxb; }

    // Lấy năm xuất bản
    int getNamXuatBan() const { return namXuatBan; }
    // Đặt năm xuất bản
    void setNamXuatBan(int nam) { namXuatBan = nam; }

    // Cho biết tài liệu có đang được mượn không
    bool getDaMuon() const { return daMuon; }
    // Đặt thẳng trạng thái mượn, không kiểm tra; việc mượn/trả bình thường nên dùng muon() và tra()
    void setDaMuon(bool tt) { daMuon = tt; }

    // Đánh dấu tài liệu đã được mượn; trả false nếu nó đang được mượn sẵn
    bool muon() {
        if (daMuon) return false;
        daMuon = true;
        return true;
    }

    // Đánh dấu tài liệu đã được trả; trả false nếu nó vốn đang ở trong kho
    bool tra() {
        if (!daMuon) return false;
        daMuon = false;
        return true;
    }

    // Tính thành tiền của tài liệu; mỗi loại có công thức riêng
    virtual double TinhTien() const = 0;

    // Tạo dòng văn bản để ghi vào file, các cột ngăn cách bởi dấu |
    virtual string xuatFile() const = 0;

    // Nhập thông tin chung từ bàn phím (mã chỉ hỏi khi chưa có), kiểm tra năm trong khoảng 1900 đến 2026
    virtual void Nhap(istream& is) {
        if (maTaiLieu.empty()) {
            cout << CYAN << "  ▸ Mã tài liệu  : " << RESET;
            getline(is >> ws, maTaiLieu);
        }
        cout << CYAN << "  ▸ Tên tài liệu : " << RESET;
        getline(is >> ws, tenTaiLieu);
        cout << CYAN << "  ▸ Nhà xuất bản : " << RESET;
        getline(is >> ws, nhaXuatBan);
        cout << CYAN << "  ▸ Năm xuất bản : " << RESET;
        while (!(is >> namXuatBan) || namXuatBan < 1900 || namXuatBan > 2026) {
            cout << RED << "    [!] Năm xuất bản không hợp lệ (1900-2026). Nhập lại: " << RESET;
            is.clear(); is.ignore(10000, '\n');
        }
    }

    // In các cột đầu của một dòng bảng (mã, tên, nhà xuất bản, năm); lớp con in tiếp các cột còn lại
    virtual void Xuat(ostream& os) const {
        os << GRAY << "│ " << RESET << WHITE << VanBan::canLe(VanBan::catChuoi(maTaiLieu, 8), 8) << RESET
           << GRAY << " │ " << RESET << VanBan::canLe(VanBan::catChuoi(tenTaiLieu, 24), 24)
           << GRAY << " │ " << RESET << VanBan::canLe(VanBan::catChuoi(nhaXuatBan, 15), 15)
           << GRAY << " │ " << RESET << setw(6) << right << to_string(namXuatBan);
    }

    // In một dòng bảng đầy đủ của tài liệu ra màn hình, kèm xuống dòng
    void HienThiThongTin() const {
        Xuat(cout);
        cout << "\n";
    }

    // Cho phép viết cin >> tailieu; gọi hàm Nhap đúng với loại thực của đối tượng
    friend istream& operator>>(istream& is, TaiLieu& tl) {
        tl.Nhap(is);
        return is;
    }

    // Cho phép viết cout << tailieu; gọi hàm Xuat đúng với loại thực của đối tượng
    friend ostream& operator<<(ostream& os, const TaiLieu& tl) {
        tl.Xuat(os);
        return os;
    }
};

// Sách: tài liệu có thêm tác giả và số trang; thành tiền tính theo số trang
class Sach : public TaiLieu {
private:
    int soTrang;
    string tacGia;

public:
    // Tạo sách rỗng, dùng khi nhập từ bàn phím
    Sach() : TaiLieu(), soTrang(0), tacGia("") {}

    // Tạo sách với đầy đủ thông tin
    Sach(string ma, string ten, string nxb, int nam, int trang, string tg, bool muon = false)
        : TaiLieu(ma, ten, nxb, nam, muon), soTrang(trang), tacGia(tg) {}

    // Lấy số trang
    int getSoTrang() const { return soTrang; }
    // Đặt số trang
    void setSoTrang(int trang) { soTrang = trang; }

    // Lấy tên tác giả
    string getTacGia() const { return tacGia; }
    // Đặt tên tác giả
    void setTacGia(string tg) { tacGia = tg; }

    // Thành tiền của sách là 1.500 đồng mỗi trang
    double TinhTien() const override { return soTrang * 1500.0; }

    // Dòng ghi file của sách, bắt đầu bằng nhãn SACH
    string xuatFile() const override {
        return "SACH|" + maTaiLieu + "|" + tenTaiLieu + "|" + nhaXuatBan + "|" +
               to_string(namXuatBan) + "|" + (daMuon ? "1" : "0") + "|" +
               to_string(soTrang) + "|" + tacGia;
    }

    // Nhập thông tin chung rồi nhập tác giả và số trang (phải lớn hơn 0)
    void Nhap(istream& is) override {
        TaiLieu::Nhap(is);
        cout << CYAN << "  ▸ Tác giả      : " << RESET;
        getline(is >> ws, tacGia);
        cout << CYAN << "  ▸ Số trang     : " << RESET;
        while (!(is >> soTrang) || soTrang <= 0) {
            cout << RED << "    [!] Số trang phải lớn hơn 0. Nhập lại: " << RESET;
            is.clear(); is.ignore(10000, '\n');
        }
        is.ignore(10000, '\n');
    }

    // In một dòng bảng của sách, cột đặc trưng gồm tác giả và số trang
    void Xuat(ostream& os) const override {
        TaiLieu::Xuat(os);
        xuatPhanDuoi(os, "Sách", CYAN, "TG: " + tacGia + " (" + to_string(soTrang) + "tr)");
    }
};

// Tạp chí: tài liệu có thêm số phát hành và tháng phát hành
class TapChi : public TaiLieu {
private:
    int soPhatHanh;
    int thangPhatHanh;

public:
    // Tạo tạp chí rỗng, dùng khi nhập từ bàn phím
    TapChi() : TaiLieu(), soPhatHanh(1), thangPhatHanh(1) {}

    // Tạo tạp chí với đầy đủ thông tin
    TapChi(string ma, string ten, string nxb, int nam, int soPH, int thangPH, bool muon = false)
        : TaiLieu(ma, ten, nxb, nam, muon), soPhatHanh(soPH), thangPhatHanh(thangPH) {}

    // Lấy số phát hành
    int getSoPhatHanh() const { return soPhatHanh; }
    // Đặt số phát hành
    void setSoPhatHanh(int soPH) { soPhatHanh = soPH; }

    // Lấy tháng phát hành
    int getThangPhatHanh() const { return thangPhatHanh; }
    // Đặt tháng phát hành
    void setThangPhatHanh(int thangPH) { thangPhatHanh = thangPH; }

    // Thành tiền của tạp chí là 45.000 đồng cộng 1.000 mỗi tháng và 500 mỗi số phát hành
    double TinhTien() const override { return 45000.0 + thangPhatHanh * 1000.0 + soPhatHanh * 500.0; }

    // Dòng ghi file của tạp chí, bắt đầu bằng nhãn TAPCHI
    string xuatFile() const override {
        return "TAPCHI|" + maTaiLieu + "|" + tenTaiLieu + "|" + nhaXuatBan + "|" +
               to_string(namXuatBan) + "|" + (daMuon ? "1" : "0") + "|" +
               to_string(soPhatHanh) + "|" + to_string(thangPhatHanh);
    }

    // Nhập thông tin chung rồi nhập số phát hành (lớn hơn 0) và tháng (từ 1 đến 12)
    void Nhap(istream& is) override {
        TaiLieu::Nhap(is);
        cout << CYAN << "  ▸ Số phát hành : " << RESET;
        while (!(is >> soPhatHanh) || soPhatHanh <= 0) {
            cout << RED << "    [!] Số phát hành phải > 0. Nhập lại: " << RESET;
            is.clear(); is.ignore(10000, '\n');
        }
        cout << CYAN << "  ▸ Tháng (1-12) : " << RESET;
        while (!(is >> thangPhatHanh) || thangPhatHanh < 1 || thangPhatHanh > 12) {
            cout << RED << "    [!] Tháng từ 1 đến 12. Nhập lại: " << RESET;
            is.clear(); is.ignore(10000, '\n');
        }
        is.ignore(10000, '\n');
    }

    // In một dòng bảng của tạp chí, cột đặc trưng gồm kỳ và tháng
    void Xuat(ostream& os) const override {
        TaiLieu::Xuat(os);
        xuatPhanDuoi(os, "Tạp chí", MAGENTA, "Kỳ #" + to_string(soPhatHanh) + " - Tháng " + to_string(thangPhatHanh));
    }
};

// Báo: tài liệu có thêm ngày phát hành dạng dd/mm/yyyy
class Bao : public TaiLieu {
private:
    string ngayPhatHanh;

public:
    // Tạo báo rỗng, dùng khi nhập từ bàn phím
    Bao() : TaiLieu(), ngayPhatHanh("01/01/2026") {}

    // Tạo báo với đầy đủ thông tin
    Bao(string ma, string ten, string nxb, int nam, string ngayPH, bool muon = false)
        : TaiLieu(ma, ten, nxb, nam, muon), ngayPhatHanh(ngayPH) {}

    // Lấy ngày phát hành
    string getNgayPhatHanh() const { return ngayPhatHanh; }
    // Đặt ngày phát hành
    void setNgayPhatHanh(string ngayPH) { ngayPhatHanh = ngayPH; }

    // Thành tiền của báo là 20.000 đồng nếu phát hành ngày 01 hoặc 15, các ngày khác là 15.000 đồng
    double TinhTien() const override {
        if (ngayPhatHanh.rfind("01/", 0) == 0 || ngayPhatHanh.rfind("15/", 0) == 0)
            return 20000.0;
        return 15000.0;
    }

    // Dòng ghi file của báo, bắt đầu bằng nhãn BAO
    string xuatFile() const override {
        return "BAO|" + maTaiLieu + "|" + tenTaiLieu + "|" + nhaXuatBan + "|" +
               to_string(namXuatBan) + "|" + (daMuon ? "1" : "0") + "|" + ngayPhatHanh;
    }

    // Nhập thông tin chung rồi nhập ngày phát hành
    void Nhap(istream& is) override {
        TaiLieu::Nhap(is);
        cout << CYAN << "  ▸ Ngày PH (dd/mm/yyyy): " << RESET;
        getline(is >> ws, ngayPhatHanh);
    }

    // In một dòng bảng của báo, cột đặc trưng là ngày phát hành
    void Xuat(ostream& os) const override {
        TaiLieu::Xuat(os);
        xuatPhanDuoi(os, "Báo", BLUE, "Ngày PH: " + ngayPhatHanh);
    }
};

// Thư viện: sở hữu danh sách tài liệu, lo việc đọc/ghi file và mọi nghiệp vụ.
// Mỗi thay đổi (thêm, xóa, mượn, trả, sắp xếp) đều được lưu vào file ngay.
class ThuVien {
public:
    // Các tiêu chí sắp xếp danh sách
    enum class TieuChiSapXep { THEO_TEN, THEO_NAM, THEO_GIA };

    // Kết quả của thao tác mượn hoặc trả
    enum class KetQua { THANH_CONG, KHONG_TIM_THAY, SAI_TRANG_THAI };

private:
    vector<TaiLieu*> ds;
    string tenFile;

    // Tạo đúng loại tài liệu từ các cột của một dòng file (cột đầu là nhãn SACH, TAPCHI hoặc BAO).
    // Trả nullptr nếu dòng thiếu cột, sai nhãn hoặc có số không hợp lệ.
    static TaiLieu* taoTuDong(const vector<string>& t) {
        if (t.size() < 7) return nullptr;
        try {
            const string& loai = t[0];
            const string& ma = t[1];
            const string& ten = t[2];
            const string& nxb = t[3];
            int nam = stoi(t[4]);
            bool muon = (t[5] == "1");

            if (loai == "SACH" && t.size() >= 8)
                return new Sach(ma, ten, nxb, nam, stoi(t[6]), t[7], muon);
            if (loai == "TAPCHI" && t.size() >= 8)
                return new TapChi(ma, ten, nxb, nam, stoi(t[6]), stoi(t[7]), muon);
            if (loai == "BAO")
                return new Bao(ma, ten, nxb, nam, t[6], muon);
        } catch (const exception&) {
            // dòng có số không hợp lệ: bỏ qua
        }
        return nullptr;
    }

    // Đọc toàn bộ file vào danh sách, bỏ qua dòng trống và dòng hỏng.
    // Trả false nếu không mở được file hoặc không đọc được tài liệu nào.
    bool docFile() {
        ifstream f(tenFile);
        if (!f.is_open()) return false;

        string dong;
        while (getline(f, dong)) {
            dong = VanBan::catKhoangTrang(dong);
            if (dong.empty()) continue;

            stringstream ss(dong);
            vector<string> cot;
            string item;
            while (getline(ss, item, '|')) cot.push_back(item);

            TaiLieu* tl = taoTuDong(cot);
            if (tl) ds.push_back(tl);
        }
        return !ds.empty();
    }

    // Thêm 12 tài liệu mẫu vào danh sách, dùng khi chưa có dữ liệu
    void taoDuLieuMau() {
        ds.push_back(new Sach("S001", "Cấu Trúc Dữ Liệu & GT", "NXB Giáo Dục", 2024, 420, "Lê Minh Hoàng", false));
        ds.push_back(new Sach("S002", "Đắc Nhân Tâm", "NXB Tổng Hợp", 2020, 300, "Dale Carnegie", true));
        ds.push_back(new Sach("S003", "Kinh Tế Vĩ Mô", "NXB Kinh Tế", 2022, 250, "Paul Samuelson", false));
        ds.push_back(new Sach("S004", "Việt Nam Sử Lược", "NXB Trẻ", 2018, 500, "Trần Trọng Kim", false));
        ds.push_back(new Sach("S005", "Nhà Giả Kim", "NXB Hội Nhà Văn", 2015, 220, "Paulo Coelho", true));
        ds.push_back(new Sach("S006", "Vũ Trụ (Cosmos)", "NXB Khoa Học", 2021, 350, "Carl Sagan", false));
        ds.push_back(new Sach("S007", "Lược Sử Loài Người", "NXB Tri Thức", 2019, 450, "Yuval Harari", false));
        ds.push_back(new Sach("S008", "Tâm Lý Học Tội Phạm", "NXB CAND", 2023, 280, "Stanton Samenow", false));
        ds.push_back(new Sach("S009", "Nghệ Thuật Bán Hàng", "NXB Trẻ", 2021, 200, "Brian Tracy", false));
        ds.push_back(new Sach("S010", "Thủy Hử", "NXB Văn Học", 2010, 600, "Thi Nại Am", false));
        ds.push_back(new TapChi("TC01", "Forbes Việt Nam", "NXB Kinh Tế", 2025, 12, 10, false));
        ds.push_back(new Bao("B001", "Báo Tuổi Trẻ", "NXB Trẻ", 2026, "15/10/2026", false));
    }

public:
    // Tạo thư viện gắn với file tenFile; chưa đọc file, hãy gọi khoiTao() sau đó
    explicit ThuVien(const string& tenFile) : tenFile(tenFile) {}

    // Giải phóng mọi tài liệu mà thư viện đang sở hữu
    ~ThuVien() {
        for (TaiLieu* tl : ds) delete tl;
    }

    // ThuVien sở hữu các con trỏ nên không cho sao chép, tránh xóa hai lần
    ThuVien(const ThuVien&) = delete;
    ThuVien& operator=(const ThuVien&) = delete;

    // Đọc dữ liệu từ file; nếu chưa có dữ liệu thì tạo dữ liệu mẫu và lưu lại
    void khoiTao() {
        if (!docFile()) {
            taoDuLieuMau();
            luuFile();
        }
    }

    // Ghi toàn bộ danh sách ra file, mỗi tài liệu một dòng
    void luuFile() const {
        ofstream f(tenFile);
        if (!f.is_open()) return;
        for (const TaiLieu* tl : ds) f << tl->xuatFile() << "\n";
    }

    // Trả danh sách tài liệu hiện có (chỉ để đọc)
    const vector<TaiLieu*>& danhSach() const { return ds; }

    // Tìm tài liệu theo mã, không phân biệt hoa thường; trả nullptr nếu không có
    TaiLieu* tim(const string& ma) const {
        string maThuong = VanBan::chuThuong(ma);
        for (TaiLieu* tl : ds) {
            if (VanBan::chuThuong(tl->getMaTaiLieu()) == maThuong) return tl;
        }
        return nullptr;
    }

    // Cho biết mã tài liệu này đã có trong thư viện chưa
    bool tonTai(const string& ma) const { return tim(ma) != nullptr; }

    // Tìm các tài liệu có từ khóa nằm trong mã hoặc tên, không phân biệt hoa thường
    vector<TaiLieu*> timKiem(const string& tuKhoa) const {
        string tk = VanBan::chuThuong(tuKhoa);
        vector<TaiLieu*> ketQua;
        for (TaiLieu* tl : ds) {
            if (VanBan::chuThuong(tl->getMaTaiLieu()).find(tk) != string::npos ||
                VanBan::chuThuong(tl->getTenTaiLieu()).find(tk) != string::npos) {
                ketQua.push_back(tl);
            }
        }
        return ketQua;
    }

    // Lọc các tài liệu theo trạng thái: daMuon = true lấy sách đang mượn, false lấy sách còn trong kho
    vector<TaiLieu*> locTheoTrangThai(bool daMuon) const {
        vector<TaiLieu*> ketQua;
        for (TaiLieu* tl : ds) {
            if (tl->getDaMuon() == daMuon) ketQua.push_back(tl);
        }
        return ketQua;
    }

    // Tổng số tài liệu trong thư viện
    int soLuong() const { return (int)ds.size(); }

    // Số tài liệu đang được mượn
    int soDangMuon() const { return (int)locTheoTrangThai(true).size(); }

    // Tổng thành tiền của mọi tài liệu trong kho
    double tongGiaTri() const {
        double tong = 0;
        for (const TaiLieu* tl : ds) tong += tl->TinhTien();
        return tong;
    }

    // Thêm tài liệu vào thư viện rồi lưu file; thư viện nhận quyền sở hữu con trỏ tl
    void them(TaiLieu* tl) {
        ds.push_back(tl);
        luuFile();
    }

    // Xóa tài liệu theo mã rồi lưu file; trả false nếu không tìm thấy
    bool xoa(const string& ma) {
        string maThuong = VanBan::chuThuong(ma);
        auto it = find_if(ds.begin(), ds.end(), [&](const TaiLieu* tl) {
            return VanBan::chuThuong(tl->getMaTaiLieu()) == maThuong;
        });
        if (it == ds.end()) return false;
        delete *it;
        ds.erase(it);
        luuFile();
        return true;
    }

    // Cho mượn tài liệu theo mã; lưu file khi thành công
    KetQua muon(const string& ma) {
        TaiLieu* tl = tim(ma);
        if (!tl) return KetQua::KHONG_TIM_THAY;
        if (!tl->muon()) return KetQua::SAI_TRANG_THAI;
        luuFile();
        return KetQua::THANH_CONG;
    }

    // Nhận trả tài liệu theo mã; lưu file khi thành công
    KetQua tra(const string& ma) {
        TaiLieu* tl = tim(ma);
        if (!tl) return KetQua::KHONG_TIM_THAY;
        if (!tl->tra()) return KetQua::SAI_TRANG_THAI;
        luuFile();
        return KetQua::THANH_CONG;
    }

    // Sắp xếp danh sách theo tiêu chí (tên A đến Z, năm mới đến cũ, giá cao đến thấp) rồi lưu file
    void sapXep(TieuChiSapXep tieuChi) {
        switch (tieuChi) {
            case TieuChiSapXep::THEO_TEN:
                sort(ds.begin(), ds.end(), [](const TaiLieu* a, const TaiLieu* b) {
                    return a->getTenTaiLieu() < b->getTenTaiLieu();
                });
                break;
            case TieuChiSapXep::THEO_NAM:
                sort(ds.begin(), ds.end(), [](const TaiLieu* a, const TaiLieu* b) {
                    return a->getNamXuatBan() > b->getNamXuatBan();
                });
                break;
            case TieuChiSapXep::THEO_GIA:
                sort(ds.begin(), ds.end(), [](const TaiLieu* a, const TaiLieu* b) {
                    return a->TinhTien() > b->TinhTien();
                });
                break;
        }
        luuFile();
    }
};

// Ứng dụng: chạy vòng lặp menu chính và nối giao diện với thư viện.
// Mỗi chức năng trong menu là một hàm riêng.
class UngDung {
private:
    ThuVien thuVien;

    // Tạo dòng thống kê hiện ở menu: tổng số, số có sẵn, số đang mượn
    string moTaThongKe() const {
        int tong = thuVien.soLuong();
        int dangMuon = thuVien.soDangMuon();
        return "Tổng: " + to_string(tong) + "  │  Có sẵn: " + to_string(tong - dangMuon) +
               "  │  Đang mượn: " + to_string(dangMuon);
    }

    // In một bảng đầy đủ (đầu bảng, từng dòng tài liệu, mép dưới) cho danh sách đã cho
    static void inBang(const vector<TaiLieu*>& danhSach) {
        GiaoDien::inTieuDeBang();
        for (const TaiLieu* tl : danhSach) tl->HienThiThongTin();
        GiaoDien::keKhungBang(2);
    }

    // Nhập mã chưa có trong thư viện, hỏi lại nếu trùng; trả false nếu người dùng hủy
    bool nhapMaMoi(string& maOut) const {
        while (true) {
            if (!GiaoDien::nhapCoThoat("Mã tài liệu", maOut)) return false;
            if (!thuVien.tonTai(maOut)) return true;
            cout << RED << "    [!] Mã '" << maOut << "' đã tồn tại! Vui lòng nhập mã khác.\n" << RESET;
        }
    }

    // Chức năng 1, 2, 3: thêm một tài liệu mới; dùng chung cho Sach, TapChi và Bao
    template <typename LoaiTaiLieu>
    void themMoi(const string& tieuDe, const string& tenLoai) {
        GiaoDien::inTieuDeChucNang(tieuDe);
        string ma;
        if (!nhapMaMoi(ma)) return;

        LoaiTaiLieu* tl = new LoaiTaiLieu();
        tl->setMaTaiLieu(ma);
        cin >> *tl;
        string ten = tl->getTenTaiLieu();
        thuVien.them(tl);
        cout << "\n";
        GiaoDien::baoThanhCong("Đã thêm " + tenLoai + " '" + ten + "' và lưu vào file!");
    }

    // Chức năng 4: in toàn bộ danh sách kèm tổng số tài liệu và tổng giá trị kho
    void xemDanhSach() {
        GiaoDien::inTieuDeChucNang("DANH SÁCH TÀI LIỆU THƯ VIỆN");
        inBang(thuVien.danhSach());
        cout << VanBan::canLe("Tổng số: " + to_string(thuVien.soLuong()) + " tài liệu", 90)
             << WHITE << VanBan::canLe("Tổng giá trị kho: ", 28, true) << RESET
             << YELLOW << VanBan::canLe(VanBan::dinhDangTien(thuVien.tongGiaTri()), 15, true) << RESET << "\n";
    }

    // Chức năng 5: tìm tài liệu theo mã hoặc tên rồi in kết quả
    void timKiem() {
        GiaoDien::inTieuDeChucNang("TRA CỨU TÀI LIỆU");
        string tuKhoa;
        if (!GiaoDien::nhapCoThoat("Nhập Mã hoặc Tên tài liệu cần tìm", tuKhoa)) return;

        vector<TaiLieu*> ketQua = thuVien.timKiem(tuKhoa);
        cout << "\n";
        inBang(ketQua);
        if (ketQua.empty()) {
            GiaoDien::baoLoi("Không tìm thấy tài liệu nào khớp với từ khóa '" + tuKhoa + "'.");
        } else {
            GiaoDien::baoThanhCong("Tìm thấy " + to_string(ketQua.size()) + " kết quả phù hợp.");
        }
    }

    // Chức năng 6: hiện các tài liệu còn trong kho rồi cho mượn theo mã người dùng nhập
    void muonTaiLieu() {
        GiaoDien::inTieuDeChucNang("ĐĂNG KÝ MƯỢN TÀI LIỆU");
        cout << YELLOW << "  Danh sách các tài liệu hiện đang có sẵn trong kho:\n" << RESET;
        vector<TaiLieu*> coSan = thuVien.locTheoTrangThai(false);
        inBang(coSan);
        if (coSan.empty()) {
            GiaoDien::baoLoi("Tất cả tài liệu hiện đều đã được cho mượn!");
            return;
        }

        cout << "\n";
        string ma;
        if (!GiaoDien::nhapCoThoat("Nhập mã tài liệu muốn mượn", ma)) return;

        switch (thuVien.muon(ma)) {
            case ThuVien::KetQua::KHONG_TIM_THAY:
                GiaoDien::baoLoi("Không tìm thấy tài liệu có mã '" + ma + "'!");
                break;
            case ThuVien::KetQua::SAI_TRANG_THAI:
                GiaoDien::baoLoi("Tài liệu '" + thuVien.tim(ma)->getTenTaiLieu() + "' hiện đang được người khác mượn!");
                break;
            case ThuVien::KetQua::THANH_CONG:
                GiaoDien::baoThanhCong("Đã xác nhận cho mượn tài liệu: " + thuVien.tim(ma)->getTenTaiLieu());
                break;
        }
    }

    // Chức năng 7: hiện các tài liệu đang được mượn rồi nhận trả theo mã người dùng nhập
    void traTaiLieu() {
        GiaoDien::inTieuDeChucNang("ĐĂNG KÝ TRẢ TÀI LIỆU");
        cout << YELLOW << "  Danh sách các tài liệu đang được độc giả mượn:\n" << RESET;
        vector<TaiLieu*> dangMuon = thuVien.locTheoTrangThai(true);
        inBang(dangMuon);
        if (dangMuon.empty()) {
            GiaoDien::baoThongTin("Hiện không có tài liệu nào đang được mượn.");
            return;
        }

        cout << "\n";
        string ma;
        if (!GiaoDien::nhapCoThoat("Nhập mã tài liệu cần trả", ma)) return;

        switch (thuVien.tra(ma)) {
            case ThuVien::KetQua::KHONG_TIM_THAY:
                GiaoDien::baoLoi("Không tìm thấy tài liệu có mã '" + ma + "'!");
                break;
            case ThuVien::KetQua::SAI_TRANG_THAI:
                GiaoDien::baoLoi("Tài liệu '" + thuVien.tim(ma)->getTenTaiLieu() + "' vốn đang ở sẵn trong kho!");
                break;
            case ThuVien::KetQua::THANH_CONG:
                GiaoDien::baoThanhCong("Đã nhận trả thành công tài liệu: " + thuVien.tim(ma)->getTenTaiLieu());
                break;
        }
    }

    // Chức năng 8: cho chọn tiêu chí sắp xếp, sắp xếp rồi in lại danh sách
    void sapXep() {
        vector<string> menuSX = {
            "1. Sắp xếp theo Tên tài liệu (A -> Z)",
            "2. Sắp xếp theo Năm xuất bản (Mới -> Cũ)",
            "3. Sắp xếp theo Giá tiền (Cao -> Thấp)",
            "4. Quay lại menu chính"
        };
        int sx = GiaoDien::chonMenu("CHỌN TIÊU CHÍ SẮP XẾP", menuSX, moTaThongKe());
        GiaoDien::xoaManHinh();
        if (sx < 0 || sx > 2) return;

        thuVien.sapXep(static_cast<ThuVien::TieuChiSapXep>(sx));
        GiaoDien::inTieuDeChucNang("DANH SÁCH SAU KHI SẮP XẾP");
        inBang(thuVien.danhSach());
    }

    // Chức năng 9: tìm tài liệu theo mã, hỏi xác nhận rồi xóa
    void xoaTaiLieu() {
        GiaoDien::inTieuDeChucNang("XÓA TÀI LIỆU KHỎI KHO");
        string ma;
        if (!GiaoDien::nhapCoThoat("Nhập mã tài liệu cần xóa", ma)) return;

        TaiLieu* tl = thuVien.tim(ma);
        if (!tl) {
            GiaoDien::baoLoi("Không tìm thấy tài liệu có mã '" + ma + "'!");
            return;
        }

        cout << "\n";
        inBang({tl});
        if (GiaoDien::hoiXacNhan("Bạn có chắc chắn muốn xóa tài liệu này?")) {
            thuVien.xoa(ma);
            GiaoDien::baoThanhCong("Đã xóa tài liệu và cập nhật file!");
        } else {
            GiaoDien::baoThongTin("Đã hủy thao tác xóa.");
        }
    }

    // Chức năng 0: lưu dữ liệu lần cuối và chào tạm biệt
    void thoat() {
        thuVien.luuFile();
        cout << GREEN << "\n  [✔] Đã lưu toàn bộ dữ liệu vào file '" << TEN_FILE << "'. Hẹn gặp lại!\n" << RESET;
    }

    // Gọi đúng chức năng theo số đã chọn trong menu (0 là thoát)
    void xuLy(int chon) {
        switch (chon) {
            case 1: themMoi<Sach>("THÊM SÁCH MỚI", "Sách"); break;
            case 2: themMoi<TapChi>("THÊM TẠP CHÍ MỚI", "Tạp chí"); break;
            case 3: themMoi<Bao>("THÊM BÁO MỚI", "Báo"); break;
            case 4: xemDanhSach(); break;
            case 5: timKiem(); break;
            case 6: muonTaiLieu(); break;
            case 7: traTaiLieu(); break;
            case 8: sapXep(); break;
            case 9: xoaTaiLieu(); break;
            case 0: thoat(); break;
        }
    }

public:
    // Tạo ứng dụng với thư viện gắn vào file TEN_FILE
    UngDung() : thuVien(TEN_FILE) {}

    // Khởi tạo console và dữ liệu, rồi lặp: hiện menu, chạy chức năng được chọn, cho đến khi chọn thoát
    void chay() {
        GiaoDien::khoiTao();
        thuVien.khoiTao();

        vector<string> menu = {
            "1. Thêm mới Sách",
            "2. Thêm mới Tạp chí",
            "3. Thêm mới Báo",
            "4. Xem toàn bộ danh sách tài liệu",
            "5. Tìm kiếm tài liệu (theo Mã hoặc Tên)",
            "6. Đăng ký mượn tài liệu",
            "7. Đăng ký trả tài liệu",
            "8. Sắp xếp danh sách tài liệu",
            "9. Xóa tài liệu khỏi thư viện",
            "0. Lưu & Thoát chương trình"
        };

        int chon = -1;
        do {
            int vt = GiaoDien::chonMenu("HỆ THỐNG QUẢN LÝ THƯ VIỆN", menu, moTaThongKe());
            chon = (vt == 9) ? 0 : (vt + 1);
            GiaoDien::xoaManHinh();

            xuLy(chon);

            if (chon == 8) GiaoDien::tamDung("tiếp tục");
            else if (chon != 0) GiaoDien::tamDung();
        } while (chon != 0);
    }
};

// Điểm vào của chương trình: tạo ứng dụng và chạy
int main() {
    UngDung app;
    app.chay();
    return 0;
}
