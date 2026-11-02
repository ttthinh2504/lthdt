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

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

const string TEN_FILE = "thuvien.txt";

const string RESET   = "\033[0m";
const string GRAY    = "\033[90m";
const string RED     = "\033[1;31m";
const string GREEN   = "\033[1;32m";
const string YELLOW  = "\033[1;33m";
const string BLUE    = "\033[1;34m";
const string MAGENTA = "\033[1;35m";
const string CYAN    = "\033[1;36m";
const string WHITE   = "\033[1;37m";
const string H_LIGHT = "\033[1;37;44m";

void initConsole() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void layKichThuocConsole(int& cols, int& rows) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        cols = 120;
        rows = 30;
    }
}

int doDaiHienThi(const string& s) {
    int count = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) count++;
    }
    return count;
}

string canLe(const string& s, int width, bool alignRight = false) {
    int actualLen = doDaiHienThi(s);
    int diff = width - actualLen;
    if (diff < 0) diff = 0;
    return alignRight ? (string(diff, ' ') + s) : (s + string(diff, ' '));
}

string catChuoi(const string& s, int maxLen) {
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

string dinhDangTien(double tien) {
    long long val = (long long)(tien + 0.5);
    string s = to_string(val);
    int n = (int)s.length() - 3;
    while (n > 0) {
        s.insert(n, ".");
        n -= 3;
    }
    return s + " đ";
}

string chuThuong(string s) {
    for (char& c : s) c = tolower((unsigned char)c);
    return s;
}

string catKhoangTrang(const string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

string lapChuoi(const string& s, int n) {
    string res = "";
    for (int i = 0; i < n; ++i) res += s;
    return res;
}

class TaiLieu {
protected:
    string maTaiLieu;
    string tenTaiLieu;
    string nhaXuatBan;
    int namXuatBan;
    bool daMuon;

public:
    TaiLieu() : maTaiLieu(""), tenTaiLieu(""), nhaXuatBan(""), namXuatBan(2026), daMuon(false) {}
    TaiLieu(string ma, string ten, string nxb, int nam, bool muon = false)
        : maTaiLieu(ma), tenTaiLieu(ten), nhaXuatBan(nxb), namXuatBan(nam), daMuon(muon) {}
    virtual ~TaiLieu() {}

    string getMaTaiLieu() const { return maTaiLieu; }
    void setMaTaiLieu(string ma) { maTaiLieu = ma; }

    string getTenTaiLieu() const { return tenTaiLieu; }
    void setTenTaiLieu(string ten) { tenTaiLieu = ten; }

    string getNhaXuatBan() const { return nhaXuatBan; }
    void setNhaXuatBan(string nxb) { nhaXuatBan = nxb; }

    int getNamXuatBan() const { return namXuatBan; }
    void setNamXuatBan(int nam) { namXuatBan = nam; }

    bool getDaMuon() const { return daMuon; }
    void setDaMuon(bool tt) { daMuon = tt; }

    virtual void HienThiThongTin() const = 0;
    virtual double TinhTien() const = 0;
    virtual string xuatFile() const = 0;

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

    virtual void Xuat(ostream& os) const {
        string ma  = catChuoi(maTaiLieu, 8);
        string ten = catChuoi(tenTaiLieu, 24);
        string nxb = catChuoi(nhaXuatBan, 15);
        string nam = to_string(namXuatBan);

        os << GRAY << "│ " << RESET << WHITE << canLe(ma, 8) << RESET
           << GRAY << " │ " << RESET << canLe(ten, 24)
           << GRAY << " │ " << RESET << canLe(nxb, 15)
           << GRAY << " │ " << RESET << setw(6) << right << nam;
    }

    friend istream& operator>>(istream& is, TaiLieu& tl) {
        tl.Nhap(is);
        return is;
    }

    friend ostream& operator<<(ostream& os, const TaiLieu& tl) {
        tl.Xuat(os);
        return os;
    }
};

class Sach : public TaiLieu {
private:
    int soTrang;
    string tacGia;

public:
    Sach() : TaiLieu(), soTrang(0), tacGia("") {}
    Sach(string ma, string ten, string nxb, int nam, int trang, string tg, bool muon = false)
        : TaiLieu(ma, ten, nxb, nam, muon), soTrang(trang), tacGia(tg) {}

    int getSoTrang() const { return soTrang; }
    void setSoTrang(int trang) { soTrang = trang; }

    string getTacGia() const { return tacGia; }
    void setTacGia(string tg) { tacGia = tg; }

    double TinhTien() const override { return soTrang * 1500.0; }

    string xuatFile() const override {
        return "SACH|" + maTaiLieu + "|" + tenTaiLieu + "|" + nhaXuatBan + "|" +
               to_string(namXuatBan) + "|" + (daMuon ? "1" : "0") + "|" +
               to_string(soTrang) + "|" + tacGia;
    }

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

    void Xuat(ostream& os) const override {
        TaiLieu::Xuat(os);
        string loai = "Sách";
        string chiTiet = catChuoi("TG: " + tacGia + " (" + to_string(soTrang) + "tr)", 25);
        string tThai = daMuon ? "Đang mượn" : "Có sẵn";
        string tien = dinhDangTien(TinhTien());

        os << GRAY << " │ " << RESET << CYAN << canLe(loai, 9) << RESET
           << GRAY << " │ " << RESET << canLe(chiTiet, 25)
           << GRAY << " │ " << RESET << (daMuon ? RED : GREEN) << canLe(tThai, 11) << RESET
           << GRAY << " │ " << RESET << YELLOW << canLe(tien, 12, true) << RESET << GRAY << " │" << RESET;
    }

    void HienThiThongTin() const override {
        cout << *this << "\n";
    }

    friend istream& operator>>(istream& is, Sach& s) { s.Nhap(is); return is; }
    friend ostream& operator<<(ostream& os, const Sach& s) { s.Xuat(os); return os; }
};

class TapChi : public TaiLieu {
private:
    int soPhatHanh;
    int thangPhatHanh;

public:
    TapChi() : TaiLieu(), soPhatHanh(1), thangPhatHanh(1) {}
    TapChi(string ma, string ten, string nxb, int nam, int soPH, int thangPH, bool muon = false)
        : TaiLieu(ma, ten, nxb, nam, muon), soPhatHanh(soPH), thangPhatHanh(thangPH) {}

    int getSoPhatHanh() const { return soPhatHanh; }
    void setSoPhatHanh(int soPH) { soPhatHanh = soPH; }

    int getThangPhatHanh() const { return thangPhatHanh; }
    void setThangPhatHanh(int thangPH) { thangPhatHanh = thangPH; }

    double TinhTien() const override { return 45000.0 + thangPhatHanh * 1000.0 + soPhatHanh * 500.0; }

    string xuatFile() const override {
        return "TAPCHI|" + maTaiLieu + "|" + tenTaiLieu + "|" + nhaXuatBan + "|" +
               to_string(namXuatBan) + "|" + (daMuon ? "1" : "0") + "|" +
               to_string(soPhatHanh) + "|" + to_string(thangPhatHanh);
    }

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

    void Xuat(ostream& os) const override {
        TaiLieu::Xuat(os);
        string loai = "Tạp chí";
        string chiTiet = catChuoi("Kỳ #" + to_string(soPhatHanh) + " - Tháng " + to_string(thangPhatHanh), 25);
        string tThai = daMuon ? "Đang mượn" : "Có sẵn";
        string tien = dinhDangTien(TinhTien());

        os << GRAY << " │ " << RESET << MAGENTA << canLe(loai, 9) << RESET
           << GRAY << " │ " << RESET << canLe(chiTiet, 25)
           << GRAY << " │ " << RESET << (daMuon ? RED : GREEN) << canLe(tThai, 11) << RESET
           << GRAY << " │ " << RESET << YELLOW << canLe(tien, 12, true) << RESET << GRAY << " │" << RESET;
    }

    void HienThiThongTin() const override {
        cout << *this << "\n";
    }

    friend istream& operator>>(istream& is, TapChi& tc) { tc.Nhap(is); return is; }
    friend ostream& operator<<(ostream& os, const TapChi& tc) { tc.Xuat(os); return os; }
};

class Bao : public TaiLieu {
private:
    string ngayPhatHanh;

public:
    Bao() : TaiLieu(), ngayPhatHanh("01/01/2026") {}
    Bao(string ma, string ten, string nxb, int nam, string ngayPH, bool muon = false)
        : TaiLieu(ma, ten, nxb, nam, muon), ngayPhatHanh(ngayPH) {}

    string getNgayPhatHanh() const { return ngayPhatHanh; }
    void setNgayPhatHanh(string ngayPH) { ngayPhatHanh = ngayPH; }

    double TinhTien() const override {
        if (ngayPhatHanh.rfind("01/", 0) == 0 || ngayPhatHanh.rfind("15/", 0) == 0)
            return 20000.0;
        return 15000.0;
    }

    string xuatFile() const override {
        return "BAO|" + maTaiLieu + "|" + tenTaiLieu + "|" + nhaXuatBan + "|" +
               to_string(namXuatBan) + "|" + (daMuon ? "1" : "0") + "|" + ngayPhatHanh;
    }

    void Nhap(istream& is) override {
        TaiLieu::Nhap(is);
        cout << CYAN << "  ▸ Ngày PH (dd/mm/yyyy): " << RESET;
        getline(is >> ws, ngayPhatHanh);
    }

    void Xuat(ostream& os) const override {
        TaiLieu::Xuat(os);
        string loai = "Báo";
        string chiTiet = catChuoi("Ngày PH: " + ngayPhatHanh, 25);
        string tThai = daMuon ? "Đang mượn" : "Có sẵn";
        string tien = dinhDangTien(TinhTien());

        os << GRAY << " │ " << RESET << BLUE << canLe(loai, 9) << RESET
           << GRAY << " │ " << RESET << canLe(chiTiet, 25)
           << GRAY << " │ " << RESET << (daMuon ? RED : GREEN) << canLe(tThai, 11) << RESET
           << GRAY << " │ " << RESET << YELLOW << canLe(tien, 12, true) << RESET << GRAY << " │" << RESET;
    }

    void HienThiThongTin() const override {
        cout << *this << "\n";
    }

    friend istream& operator>>(istream& is, Bao& b) { b.Nhap(is); return is; }
    friend ostream& operator<<(ostream& os, const Bao& b) { b.Xuat(os); return os; }
};

void luuDuLieuFile(const vector<TaiLieu*>& ds) {
    ofstream f(TEN_FILE);
    if (!f.is_open()) return;
    for (const TaiLieu* tl : ds) {
        f << tl->xuatFile() << "\n";
    }
    f.close();
}

bool docDuLieuFile(vector<TaiLieu*>& ds) {
    ifstream f(TEN_FILE);
    if (!f.is_open()) return false;

    string dong;
    while (getline(f, dong)) {
        dong = catKhoangTrang(dong);
        if (dong.empty()) continue;

        stringstream ss(dong);
        vector<string> t;
        string item;
        while (getline(ss, item, '|')) {
            t.push_back(item);
        }
        if (t.size() < 7) continue;

        string loai = t[0];
        string ma = t[1];
        string ten = t[2];
        string nxb = t[3];
        int nam = stoi(t[4]);
        bool muon = (t[5] == "1");

        if (loai == "SACH" && t.size() >= 8) {
            ds.push_back(new Sach(ma, ten, nxb, nam, stoi(t[6]), t[7], muon));
        } else if (loai == "TAPCHI" && t.size() >= 8) {
            ds.push_back(new TapChi(ma, ten, nxb, nam, stoi(t[6]), stoi(t[7]), muon));
        } else if (loai == "BAO" && t.size() >= 7) {
            ds.push_back(new Bao(ma, ten, nxb, nam, t[6], muon));
        }
    }
    f.close();
    return !ds.empty();
}

void keKhungBang(int viTri) {
    string l = (viTri == 0) ? "┌" : (viTri == 1 ? "├" : "└");
    string m = (viTri == 0) ? "┬" : (viTri == 1 ? "┼" : "┴");
    string r = (viTri == 0) ? "┐" : (viTri == 1 ? "┤" : "┘");

    cout << GRAY << l
         << lapChuoi("─", 10) << m
         << lapChuoi("─", 26) << m
         << lapChuoi("─", 17) << m
         << lapChuoi("─", 8)  << m
         << lapChuoi("─", 11) << m
         << lapChuoi("─", 27) << m
         << lapChuoi("─", 13) << m
         << lapChuoi("─", 14) << r << RESET << "\n";
}

void inTieuDeBang() {
    keKhungBang(0);
    cout << GRAY << "│ " << YELLOW << canLe("Mã TL", 8)
         << GRAY << " │ " << YELLOW << canLe("Tên tài liệu", 24)
         << GRAY << " │ " << YELLOW << canLe("Nhà xuất bản", 15)
         << GRAY << " │ " << YELLOW << canLe("Năm", 6, true)
         << GRAY << " │ " << YELLOW << canLe("Phân loại", 9)
         << GRAY << " │ " << YELLOW << canLe("Thông tin đặc trưng", 25)
         << GRAY << " │ " << YELLOW << canLe("Trạng thái", 11)
         << GRAY << " │ " << YELLOW << canLe("Thành tiền", 12, true)
         << GRAY << " │" << RESET << "\n";
    keKhungBang(1);
}

void inTieuDeChucNang(const string& tieuDe) {
    int w = 58;
    int pad = w - doDaiHienThi(tieuDe);
    int leftP = pad / 2, rightP = pad - leftP;
    cout << CYAN << "╔" << lapChuoi("═", w) << "╗\n"
         << "║" << YELLOW << string(leftP, ' ') << tieuDe << string(rightP, ' ') << CYAN << "║\n"
         << "╚" << lapChuoi("═", w) << "╝\n\n" << RESET;
}

TaiLieu* timTheoMa(const vector<TaiLieu*>& ds, const string& ma) {
    string maThuong = chuThuong(ma);
    for (TaiLieu* tl : ds) {
        if (chuThuong(tl->getMaTaiLieu()) == maThuong) return tl;
    }
    return nullptr;
}

bool nhapCoThoat(const string& loiNhac, string& ketQua) {
    cout << CYAN << "  ▸ " << loiNhac << GRAY << " (Nhấn Enter hoặc nhập 0 để hủy): " << RESET;
    getline(cin, ketQua);
    ketQua = catKhoangTrang(ketQua);
    if (ketQua.empty() || ketQua == "0") {
        cout << YELLOW << "  [i] Đã hủy thao tác.\n" << RESET;
        return false;
    }
    return true;
}

bool nhapMaMoi(const vector<TaiLieu*>& ds, string& maOut) {
    while (true) {
        if (!nhapCoThoat("Mã tài liệu", maOut)) return false;
        if (timTheoMa(ds, maOut) != nullptr) {
            cout << RED << "    [!] Mã '" << maOut << "' đã tồn tại! Vui lòng nhập mã khác.\n" << RESET;
        } else {
            return true;
        }
    }
}

int chonMenu(const string& tieuDe, const vector<string>& muc, const vector<TaiLieu*>& ds) {
    int chon = 0, n = (int)muc.size();
    int w = 58;
    system("cls");
    cout << "\033[?25l";

    int dangMuon = 0;
    for (const TaiLieu* tl : ds) if (tl->getDaMuon()) dangMuon++;
    int coSan = (int)ds.size() - dangMuon;
    string thongKe = "Tổng: " + to_string(ds.size()) + "  │  Có sẵn: " + to_string(coSan) + "  │  Đang mượn: " + to_string(dangMuon);

    while (true) {
        int cols, rows;
        layKichThuocConsole(cols, rows);
        int chieuCaoMenu = n + 8;
        int leTren = max(1, (rows - chieuCaoMenu) / 2);
        int leTrai = max(0, (cols - (w + 2)) / 2);
        string padLeft(leTrai, ' ');

        cout << "\033[H" << string(leTren, '\n');
        cout << padLeft << CYAN << "╔" << lapChuoi("═", w) << "╗\n";

        int padT = w - doDaiHienThi(tieuDe);
        int lT = padT / 2, rT = padT - lT;
        cout << padLeft << "║" << YELLOW << string(lT, ' ') << tieuDe << string(rT, ' ') << CYAN << "║\n";

        cout << padLeft << "╠" << lapChuoi("═", w) << "╣\n";
        int padS = w - doDaiHienThi(thongKe);
        int lS = padS / 2, rS = padS - lS;
        cout << padLeft << "║" << GREEN << string(lS, ' ') << thongKe << string(rS, ' ') << CYAN << "║\n";
        cout << padLeft << "╠" << lapChuoi("─", w) << "╣\n" << RESET;

        for (int i = 0; i < n; ++i) {
            string dong = (i == chon ? "  ❯ " : "    ") + muc[i];
            cout << padLeft << CYAN << "║ " << RESET;
            if (i == chon) {
                cout << H_LIGHT << canLe(dong, w - 2) << RESET;
            } else {
                cout << WHITE << canLe(dong, w - 2) << RESET;
            }
            cout << CYAN << " ║\n" << RESET;
        }

        cout << padLeft << CYAN << "╠" << lapChuoi("─", w) << "╣\n";
        string hd = "[↑/↓] Di chuyển   [Enter] Chọn   [Esc] Thoát";
        int padH = w - doDaiHienThi(hd);
        int lH = padH / 2, rH = padH - lH;
        cout << padLeft << "║" << GRAY << string(lH, ' ') << hd << string(rH, ' ') << CYAN << "║\n";
        cout << padLeft << "╚" << lapChuoi("═", w) << "╝\n" << RESET;

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

void tamDung() {
    cout << GRAY << "\n  Nhấn phím bất kỳ để quay về menu..." << RESET;
    _getch();
}

void taoDuLieuMau(vector<TaiLieu*>& ds) {
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

int main() {
    initConsole();
    vector<TaiLieu*> thuVien;

    if (!docDuLieuFile(thuVien)) {
        taoDuLieuMau(thuVien);
        luuDuLieuFile(thuVien);
    }

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
        int vt = chonMenu("HỆ THỐNG QUẢN LÝ THƯ VIỆN", menu, thuVien);
        chon = (vt == 9) ? 0 : (vt + 1);
        system("cls");

        switch (chon) {
            case 1: {
                inTieuDeChucNang("THÊM SÁCH MỚI");
                string ma;
                if (!nhapMaMoi(thuVien, ma)) break;
                Sach* s = new Sach();
                s->setMaTaiLieu(ma);
                cin >> *s;
                thuVien.push_back(s);
                luuDuLieuFile(thuVien);
                cout << GREEN << "\n  [✔] Đã thêm Sách '" << s->getTenTaiLieu() << "' và lưu vào file!\n" << RESET;
                break;
            }
            case 2: {
                inTieuDeChucNang("THÊM TẠP CHÍ MỚI");
                string ma;
                if (!nhapMaMoi(thuVien, ma)) break;
                TapChi* tc = new TapChi();
                tc->setMaTaiLieu(ma);
                cin >> *tc;
                thuVien.push_back(tc);
                luuDuLieuFile(thuVien);
                cout << GREEN << "\n  [✔] Đã thêm Tạp chí '" << tc->getTenTaiLieu() << "' và lưu vào file!\n" << RESET;
                break;
            }
            case 3: {
                inTieuDeChucNang("THÊM BÁO MỚI");
                string ma;
                if (!nhapMaMoi(thuVien, ma)) break;
                Bao* b = new Bao();
                b->setMaTaiLieu(ma);
                cin >> *b;
                thuVien.push_back(b);
                luuDuLieuFile(thuVien);
                cout << GREEN << "\n  [✔] Đã thêm Báo '" << b->getTenTaiLieu() << "' và lưu vào file!\n" << RESET;
                break;
            }
            case 4: {
                inTieuDeChucNang("DANH SÁCH TÀI LIỆU THƯ VIỆN");
                inTieuDeBang();
                double tong = 0;
                for (const TaiLieu* tl : thuVien) {
                    tl->HienThiThongTin();
                    tong += tl->TinhTien();
                }
                keKhungBang(2);
                cout << canLe("Tổng số: " + to_string(thuVien.size()) + " tài liệu", 90)
                     << WHITE << canLe("Tổng giá trị kho: ", 28, true) << RESET
                     << YELLOW << canLe(dinhDangTien(tong), 15, true) << RESET << "\n";
                break;
            }
            case 5: {
                inTieuDeChucNang("TRA CỨU TÀI LIỆU");
                string tuKhoa;
                if (!nhapCoThoat("Nhập Mã hoặc Tên tài liệu cần tìm", tuKhoa)) break;
                string tkThuong = chuThuong(tuKhoa);

                cout << "\n";
                inTieuDeBang();
                int dem = 0;
                for (const TaiLieu* tl : thuVien) {
                    if (chuThuong(tl->getMaTaiLieu()).find(tkThuong) != string::npos ||
                        chuThuong(tl->getTenTaiLieu()).find(tkThuong) != string::npos) {
                        tl->HienThiThongTin();
                        dem++;
                    }
                }
                keKhungBang(2);
                if (dem == 0) {
                    cout << RED << "  [!] Không tìm thấy tài liệu nào khớp với từ khóa '" << tuKhoa << "'.\n" << RESET;
                } else {
                    cout << GREEN << "  [✔] Tìm thấy " << dem << " kết quả phù hợp.\n" << RESET;
                }
                break;
            }
            case 6: {
                inTieuDeChucNang("ĐĂNG KÝ MƯỢN TÀI LIỆU");
                cout << YELLOW << "  Danh sách các tài liệu hiện đang có sẵn trong kho:\n" << RESET;
                inTieuDeBang();
                int coSan = 0;
                for (const TaiLieu* tl : thuVien) {
                    if (!tl->getDaMuon()) {
                        tl->HienThiThongTin();
                        coSan++;
                    }
                }
                keKhungBang(2);

                if (coSan == 0) {
                    cout << RED << "  [!] Tất cả tài liệu hiện đều đã được cho mượn!\n" << RESET;
                    break;
                }

                cout << "\n";
                string ma;
                if (!nhapCoThoat("Nhập mã tài liệu muốn mượn", ma)) break;

                TaiLieu* tl = timTheoMa(thuVien, ma);
                if (!tl) {
                    cout << RED << "  [!] Không tìm thấy tài liệu có mã '" << ma << "'!\n" << RESET;
                } else if (tl->getDaMuon()) {
                    cout << RED << "  [!] Tài liệu '" << tl->getTenTaiLieu() << "' hiện đang được người khác mượn!\n" << RESET;
                } else {
                    tl->setDaMuon(true);
                    luuDuLieuFile(thuVien);
                    cout << GREEN << "  [✔] Đã xác nhận cho mượn tài liệu: " << tl->getTenTaiLieu() << "\n" << RESET;
                }
                break;
            }
            case 7: {
                inTieuDeChucNang("ĐĂNG KÝ TRẢ TÀI LIỆU");
                cout << YELLOW << "  Danh sách các tài liệu đang được độc giả mượn:\n" << RESET;
                inTieuDeBang();
                int dangMuon = 0;
                for (const TaiLieu* tl : thuVien) {
                    if (tl->getDaMuon()) {
                        tl->HienThiThongTin();
                        dangMuon++;
                    }
                }
                keKhungBang(2);

                if (dangMuon == 0) {
                    cout << GREEN << "  [i] Hiện không có tài liệu nào đang được mượn.\n" << RESET;
                    break;
                }

                cout << "\n";
                string ma;
                if (!nhapCoThoat("Nhập mã tài liệu cần trả", ma)) break;

                TaiLieu* tl = timTheoMa(thuVien, ma);
                if (!tl) {
                    cout << RED << "  [!] Không tìm thấy tài liệu có mã '" << ma << "'!\n" << RESET;
                } else if (!tl->getDaMuon()) {
                    cout << RED << "  [!] Tài liệu '" << tl->getTenTaiLieu() << "' vốn đang ở sẵn trong kho!\n" << RESET;
                } else {
                    tl->setDaMuon(false);
                    luuDuLieuFile(thuVien);
                    cout << GREEN << "  [✔] Đã nhận trả thành công tài liệu: " << tl->getTenTaiLieu() << "\n" << RESET;
                }
                break;
            }
            case 8: {
                vector<string> menuSX = {
                    "1. Sắp xếp theo Tên tài liệu (A -> Z)",
                    "2. Sắp xếp theo Năm xuất bản (Mới -> Cũ)",
                    "3. Sắp xếp theo Giá tiền (Cao -> Thấp)",
                    "4. Quay lại menu chính"
                };
                int sx = chonMenu("CHỌN TIÊU CHÍ SẮP XẾP", menuSX, thuVien);
                system("cls");
                if (sx == 0) {
                    sort(thuVien.begin(), thuVien.end(), [](const TaiLieu* a, const TaiLieu* b) {
                        return a->getTenTaiLieu() < b->getTenTaiLieu();
                    });
                } else if (sx == 1) {
                    sort(thuVien.begin(), thuVien.end(), [](const TaiLieu* a, const TaiLieu* b) {
                        return a->getNamXuatBan() > b->getNamXuatBan();
                    });
                } else if (sx == 2) {
                    sort(thuVien.begin(), thuVien.end(), [](const TaiLieu* a, const TaiLieu* b) {
                        return a->TinhTien() > b->TinhTien();
                    });
                }
                if (sx >= 0 && sx <= 2) {
                    luuDuLieuFile(thuVien);
                    inTieuDeChucNang("DANH SÁCH SAU KHI SẮP XẾP");
                    inTieuDeBang();
                    for (const TaiLieu* tl : thuVien) tl->HienThiThongTin();
                    keKhungBang(2);
                }
                break;
            }
            case 9: {
                inTieuDeChucNang("XÓA TÀI LIỆU KHỎI KHO");
                string ma;
                if (!nhapCoThoat("Nhập mã tài liệu cần xóa", ma)) break;
                string maThuong = chuThuong(ma);

                auto it = find_if(thuVien.begin(), thuVien.end(), [&](const TaiLieu* tl) {
                    return chuThuong(tl->getMaTaiLieu()) == maThuong;
                });

                if (it == thuVien.end()) {
                    cout << RED << "  [!] Không tìm thấy tài liệu có mã '" << ma << "'!\n" << RESET;
                } else {
                    cout << "\n";
                    inTieuDeBang();
                    (*it)->HienThiThongTin();
                    keKhungBang(2);
                    cout << YELLOW << "\n  Bạn có chắc chắn muốn xóa tài liệu này? (y/n): " << RESET;
                    char xacNhan = _getch();
                    cout << xacNhan << "\n";
                    if (xacNhan == 'y' || xacNhan == 'Y') {
                        delete *it;
                        thuVien.erase(it);
                        luuDuLieuFile(thuVien);
                        cout << GREEN << "  [✔] Đã xóa tài liệu và cập nhật file!\n" << RESET;
                    } else {
                        cout << YELLOW << "  [i] Đã hủy thao tác xóa.\n" << RESET;
                    }
                }
                break;
            }
            case 0: {
                luuDuLieuFile(thuVien);
                cout << GREEN << "\n  [✔] Đã lưu toàn bộ dữ liệu vào file '" << TEN_FILE << "'. Hẹn gặp lại!\n" << RESET;
                break;
            }
        }

        if (chon != 0 && chon != 8) tamDung();
        else if (chon == 8) {
            cout << GRAY << "\n  Nhấn phím bất kỳ để tiếp tục..." << RESET;
            _getch();
        }

    } while (chon != 0);

    for (TaiLieu* tl : thuVien) delete tl;
    return 0;
}