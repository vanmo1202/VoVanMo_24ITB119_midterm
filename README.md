# Midterm Project – Implement `ls(1)`

## 1. Thông tin sinh viên

- **Họ tên:** VÕ VĂN MƠ
- **Mã sinh viên:** 24ITB119
- **Lớp:** Lập Trình Hệ Thống 5
- **GitHub repository:** https://github.com/vanmo1202/VoVanMo_24ITB119_midterm

## 2. Mục tiêu dự án

Dự án xây dựng một phiên bản rút gọn của lệnh UNIX `ls(1)` bằng ngôn ngữ C, bám theo manual page được cung cấp trong đề Midterm.

Chương trình sử dụng các system/library interfaces phổ biến trên UNIX/Linux như:

- `opendir()`, `readdir()`, `closedir()` để duyệt thư mục.
- `stat()`, `lstat()` để lấy metadata của file.
- `readlink()` để đọc symbolic link.
- `getpwuid()`, `getgrgid()` để lấy tên owner/group.
- `getopt()` để phân tích command-line options.
- `qsort()` để sắp xếp kết quả.

## 3. Các chức năng đã cài đặt

Cú pháp được hỗ trợ:

```text
myls [-AacdFfhiklnqRrSstuw] [file ...]
```

Các option:

| Option | Chức năng |
|---|---|
| `-A` | Hiển thị các entry ẩn, trừ `.` và `..`. Tự động bật cho super-user. |
| `-a` | Hiển thị tất cả entry, kể cả `.` và `..`. |
| `-c` | Dùng thời gian thay đổi trạng thái file cho `-t` hoặc `-l`. |
| `-d` | Liệt kê directory như file thường và không dereference symlink trong operand. |
| `-F` | Gắn ký hiệu phân loại sau pathname: `/`, `*`, `@`, `=`, `|` và `%` nếu hệ thống hỗ trợ whiteout. |
| `-f` | Không sắp xếp output. |
| `-h` | Hiển thị size/block theo dạng human-readable; override `-k` nếu xuất hiện sau. |
| `-i` | Hiển thị inode number. |
| `-k` | Hiển thị block theo đơn vị 1024 bytes; override `-h` nếu xuất hiện sau. |
| `-l` | Hiển thị long format với mode, links, owner, group, size, time và pathname. |
| `-n` | Giống `-l` nhưng owner/group được hiển thị bằng UID/GID số. |
| `-q` | Thay ký tự không in được trong tên file bằng `?`. |
| `-R` | Liệt kê thư mục con đệ quy. |
| `-r` | Đảo ngược thứ tự sort. |
| `-S` | Sort theo kích thước, lớn nhất trước. |
| `-s` | Hiển thị số block filesystem thực tế được sử dụng. |
| `-t` | Sort theo thời gian, mới nhất trước. |
| `-u` | Dùng thời gian truy cập cho `-t` hoặc `-l`. |
| `-w` | In raw các ký tự trong tên file. |

Các quan hệ override theo manual cũng được xử lý:

- `-h` và `-k`: option xuất hiện sau có hiệu lực.
- `-l` và `-n`: option xuất hiện sau quyết định hiển thị tên hay numeric UID/GID.
- `-c` và `-u`: option xuất hiện sau quyết định loại thời gian.
- `-R` và `-d`: option xuất hiện sau có hiệu lực.
- `-q` và `-w`: option xuất hiện sau quyết định cách in ký tự không printable.

Ngoài ra chương trình hỗ trợ:

- Không truyền operand thì liệt kê thư mục hiện tại `.`.
- File operand được in trước directory operand.
- File và directory operands được sort riêng.
- Sort lexicographical mặc định.
- Symlink trong directory được xử lý bằng `lstat()` và `-l` hiển thị `-> target`.
- Character/block device hiển thị major/minor number trong long format.
- Permission bits gồm setuid, setgid và sticky bit (`s`, `S`, `t`, `T`).
- `BLOCKSIZE` được dùng khi hiển thị block nếu không có `-h` hoặc `-k`.
- `TZ` được hệ thống C library sử dụng khi chuyển đổi thời gian.
- Exit status `0` khi thành công và `> 0` khi có lỗi.
- Xử lý file không tồn tại, permission error, directory rỗng và lỗi allocation mà không gây segmentation fault.

## 4. Cấu trúc project

```text
VoVanMo_24ITB119_midterm/
├── .gitignore
├── Makefile
├── README.md
├── include/
│   ├── fileinfo.h
│   ├── format.h
│   ├── listing.h
│   ├── options.h
│   └── sort.h
└── src/
    ├── fileinfo.c
    ├── format.c
    ├── listing.c
    ├── main.c
    ├── options.c
    └── sort.c
```

### Vai trò từng module

- `main.c`: điểm bắt đầu chương trình.
- `options.c/.h`: phân tích option bằng `getopt()` và lưu cấu hình chạy.
- `fileinfo.c/.h`: quản lý metadata và danh sách file.
- `listing.c/.h`: đọc directory, xử lý operands và recursive listing.
- `sort.c/.h`: thực hiện sort theo name, size hoặc time.
- `format.c/.h`: in inode, block count, long format, permission, owner/group, symlink và suffix `-F`.

## 5. Môi trường phát triển

Project được phát triển và kiểm tra trên Linux với:

- Visual Studio Code
- GCC
- GNU Make
- C11 / POSIX interfaces

## 6. Biên dịch

Tại thư mục gốc của project:

```bash
make
```

File thực thi được tạo ra:

```text
myls
```

Xóa binary, object files và dependency files:

```bash
make clean
```

## 7. Cách chạy

Liệt kê thư mục hiện tại:

```bash
./myls
```

Liệt kê một thư mục:

```bash
./myls src
```

Long format:

```bash
./myls -l
```

Hiển thị hidden files:

```bash
./myls -a
./myls -A
```

Long format + human-readable + inode:

```bash
./myls -lhi
```

Sort theo size:

```bash
./myls -S
```

Sort theo time và đảo ngược:

```bash
./myls -tr
```

Recursive:

```bash
./myls -R src
```

Nhiều operands:

```bash
./myls README.md src include
```

## 8. Ví dụ kiểm tra

Tạo dữ liệu test:

```bash
mkdir -p test_ls/sub
touch test_ls/apple.txt
touch test_ls/zebra.txt
touch test_ls/.hidden
ln -s apple.txt test_ls/link
```

So sánh output cơ bản:

```bash
./myls test_ls
ls -1 test_ls
```

Kiểm tra exit status khi file không tồn tại:

```bash
./myls does-not-exist
echo $?
```

Giá trị exit status phải lớn hơn `0`.

## 9. Kiểm tra chất lượng code

Makefile bật các warning quan trọng:

```text
-Wall -Wextra -Wpedantic
```

Project đã được kiểm tra với AddressSanitizer và UndefinedBehaviorSanitizer cho các nhóm chức năng chính để phát hiện lỗi memory/undefined behavior.

## 10. Ghi chú về khác biệt nền tảng

Manual được cung cấp là manual của NetBSD, trong khi project được build trên Linux/Ubuntu. Các file type chuẩn như regular file, directory, symlink, block/character device, socket và FIFO được hỗ trợ đầy đủ.

Whiteout chỉ được nhận diện khi nền tảng cung cấp macro `S_ISWHT`. Các trạng thái archive đặc thù của NetBSD không được Linux `stat(2)` cung cấp trực tiếp, vì vậy không thể tái tạo chúng một cách portable trên Ubuntu chỉ bằng POSIX/Linux metadata.

Project chỉ triển khai các option nằm trong manual của đề bài và không cố gắng sao chép toàn bộ GNU `ls`.
