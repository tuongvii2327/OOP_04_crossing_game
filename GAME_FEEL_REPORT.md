# TỔNG HỢP KỸ THUẬT VÀ BÁO CÁO TRIỂN KHAI GAME FEEL (TOP-DOWN 3/4 VIEW)

Tài liệu này tổng hợp các kỹ thuật tạo cảm giác chuyển động (Game Feel) chân thực và tối ưu cho game góc nhìn nghiêng 3/4 (Top-down 3/4 View / Oblique Orthographic 2.5D), cùng bản báo cáo chi tiết về thuật toán triển khai vào mã nguồn dự án.

---

## PHẦN I: CÁC KỸ THUẬT CHUYỂN ĐỘNG CƠ BẢN (PHÙ HỢP CHO 3/4 VIEW)

Ở góc nhìn 3/4 View, camera đặt chếch từ trên xuống khiến **trục độ cao $Z$ chiếu trực tiếp lên trục $Y$ dọc của màn hình**. Để tạo cú nhảy sống động và chân thực, ta kết hợp các kỹ thuật sau:

### 1. Hệ Thống Trục Z Giả & Bóng Động (Fake Z-Axis & Dynamic Shadow)
Trong game 2D (SFML), không có không gian 3D thật. "Độ cao Z" là một biến số giả lập đại diện cho khoảng cách nảy lên không trung (tính bằng pixel):
* **Tọa độ mặt đất ($Ground_{X,Y}$):** Tọa độ thực trên bản đồ, được sử dụng cho hệ thống Hitbox kiểm tra va chạm và làm điểm neo cho bóng đổ.
* **Vị trí hiển thị ($Render_Y$):** Sprite nhân vật được đẩy lên phía trên màn hình một khoảng tương ứng với độ cao $Z$ ($Render_Y = Ground_Y - Z$).
* **Bóng đổ động (Dynamic Shadow):** Đặt cố định tại mặt đất $(Ground_X, Ground_Y)$, co nhỏ lại và mờ dần khi nhân vật nhảy lên cao ($Z$ tăng), to và đậm dần khi rơi xuống.

### 2. Đường Cong Chuyển Động (Easing & Hang Time)
Áp dụng quỹ đạo Parabola hoặc hàm Sine để giả lập gia tốc trọng lực thay vì chuyển động tuyến tính. Nhân vật vọt lên nhanh, lơ lửng chậm lại ở đỉnh cú nhảy (hang time), và rơi nhanh xuống đất.

### 3. Co Giãn Đàn Hồi (Squash and Stretch)
Nguyên tắc bảo toàn thể tích kinh điển trong hoạt hình để tăng độ sống động (Juice):
* **Anticipation (Lấy đà):** Dẹp dẹp chiều cao, phình nhẹ chiều ngang khi bắt đầu nén phím nhảy.
* **Ascend/Air (Bay bổng):** Giãn dài chiều cao theo hướng vọt lên.
* **Impact (Chạm đất):** Dẹp ngang và nén chiều cao khi tiếp đất để tiêu tán lực.
* **Recovery (Hồi phục):** Phục hồi mượt mà về hình dáng gốc.

### 4. Phản Hồi Môi Trường (Juice / Screen Shake & Particles)
* **Bụi tiếp đất:** Thêm các hạt bụi (particles) văng ra 2 bên gầm chân khi nhân vật va chạm mặt đất.
* **Screen Shake:** Rung nhẹ camera khi hạ cánh để nhấn mạnh sức nặng của nhân vật.

---

## PHẦN II: BÁO CÁO PHÂN TÍCH VÀ CHI TIẾT THUẬT TOÁN TRIỂN KHAI

Phương án tích hợp chi tiết các kỹ thuật trên vào mã nguồn SFML (kết hợp bảo toàn hệ thống Hitbox và Origin `(0, 0)`):

### 1. Phân Tách Tọa Độ Mặt Đất & Trục Z Giả Lập
* **Tiến trình tổng:** Biến $progress \in [0, 1]$ ($progress = \frac{t_{\text{elapsed}}}{t_{\text{duration}}}$ với $t_{\text{duration}} = 0.12\text{s} - 0.15\text{s}$). Dùng để điều hướng Animation và độ co giãn.
* **Thời gian bay thực tế ($air\_t$):** Chỉ cất cánh khi $progress \ge 0.15$ và hạ cánh khi $progress \ge 0.85$ để chống trượt chân:
  $$air\_t = \text{clamp}\left(\frac{progress - 0.15}{0.70}, 0.0, 1.0\right)$$
* **Tọa độ mặt đất (Ground Position):** Tịnh tiến theo hàm Ease-Out Quad ($t_{\text{move}}$) để vọt nhanh dứt khoát, không gây Input Lag:
  $$t_{\text{move}} = 1 - (1 - air\_t)^2$$
  $$Ground_X = start\_pos.x + (end\_pos.x - start\_pos.x) \cdot t_{\text{move}}$$
  $$Ground_Y = start\_pos.y + (end\_pos.y - start\_pos.y) \cdot t_{\text{move}}$$
* **Độ cao Z giả lập (Fake Z Arc):** Sử dụng sóng Half-Sine:
  $$Z = H_{\text{max}} \cdot \sin(\pi \cdot air\_t)$$
  *(Với $H_{\text{max}} = 65\text{px}$ là độ cao đỉnh cú nhảy).*

### 2. Co Giãn Đàn Hồi (Squash & Stretch Interpolation)
Tỷ lệ biến đổi Scale theo từng giai đoạn của $progress$ cần đồng bộ tuyệt đối với thời gian bay:
* **$progress < 0.15$ (Anticipation - Lấy đà):** Đang đứng yên trên đất. Nén người xuống: $\text{Squash}_X = 1.15$, $\text{Squash}_Y = 0.85$.
* **$0.15 \le progress < 0.85$ (Ascend/Air - Bay bổng):** Đây là lúc $air\_t \in [0, 1]$. Nhân vật rời mặt đất, vươn dài cơ thể: $\text{Squash}_X = 0.90$, $\text{Squash}_Y = 1.15$.
* **$0.85 \le progress < 0.95$ (Impact - Chạm đất):** Vừa hạ cánh xong ($air\_t = 1$). Cơ thể bị dẹp ngang do lực va đập: $\text{Squash}_X = 1.20$, $\text{Squash}_Y = 0.80$.
* **$progress \ge 0.95$ (Recovery - Hồi phục):** Nhảy về hình dáng gốc $\text{Squash}_X = 1.00$, $\text{Squash}_Y = 1.00$.

### 3. Bù Trừ Điểm Neo Render (Render Anchor Offset)
Trong dự án, `Origin` của Sprite bị khóa tại góc trên bên trái `(0, 0)` để bảo toàn Hitbox. Do đó, việc Scale chiều cao làm nhân vật bị co dẹp từ đỉnh đầu xuống. Thuật toán bù trừ sẽ tính độ chênh lệch kích thước và giữ cố định gầm chân nhân vật tại $Ground_Y$:
* **Tỷ lệ Scale tổng hợp:**
  $$\text{Scale}_{\text{final\_x}} = \text{Scale}_{\text{base\_x}} \cdot \text{Squash}_X$$
  $$\text{Scale}_{\text{final\_y}} = \text{Scale}_{\text{base\_y}} \cdot \text{Squash}_Y$$
* **Độ chênh lệch kích thước so với khung gốc:**
  $$\Delta W = \text{FRAME\_W} \cdot (\text{Scale}_{\text{final\_x}} - \text{Scale}_{\text{base\_x}})$$
  $$\Delta H = \text{FRAME\_H} \cdot (\text{Scale}_{\text{final\_y}} - \text{Scale}_{\text{base\_y}})$$
* **Tọa độ hiển thị thực tế trên màn hình ($Render_X, Render_Y$):** Bù trừ nửa chiều rộng để giữ tâm ngang, và bù trừ chiều cao để gầm chân bám chặt mặt đất $Ground_Y$, đồng thời nâng nhân vật lên theo độ cao $Z$:
  $$\text{Render}_X = Ground_X - \frac{\Delta W}{2}$$
  $$\text{Render}_Y = Ground_Y - \Delta H - Z$$

### 4. Hệ Thống Bóng Đổ Động (Dynamic Shadow System)
Tạo đối tượng Elip bóng đổ riêng biệt và render trước khi render nhân vật:
* **Vị trí:** Luôn neo cố định tại tọa độ mặt đất $(Ground_X + \text{Offset}_X, Ground_Y + \text{Offset}_Y)$.
* **Tỷ lệ thu nhỏ:** $\text{Scale}_{\text{shadow}} = \frac{1.0}{1.0 + 0.005 \cdot Z}$
* **Độ mờ (Alpha):** Giảm dần từ 120 về 30 theo độ cao $Z$.

---

## PHẦN III: KẾ HOẠCH TÁI CẤU TRÚC CHUYỂN ĐỘNG CHO LỚP PEOPLE (GÓC NHÌN 3/4 TOP-DOWN)

Dưới đây là kế hoạch chi tiết từng bước để tích hợp bộ thuật toán chuyển động chuẩn xác cho không gian 2.5D (Top-down 3/4) của dự án. Kế hoạch này tập trung vào luồng logic và kiến trúc, không bao gồm mã nguồn cụ thể.

---

### I. MỤC TIÊU KỸ THUẬT CỐT LÕI

* **Triệt tiêu Input Lag (Độ trễ phím):** Nhân vật phản hồi tức thì bằng cách áp dụng hàm nội suy phi tuyến tính (Easing) lên mặt đất thay vì di chuyển tuyến tính đều đều.
* **Mô phỏng không gian 3D:** Tạo ảo giác về chiều cao bằng cách tách biệt tọa độ hiển thị của hình ảnh nhân vật (nhô lên trên) và bóng đổ (neo dưới đất).
* **Bảo toàn kiến trúc vật lý:** Khóa chặt hệ thống Hitbox và giữ nguyên điểm neo gốc $Origin(0,0)$ của lớp `DynamicObject`, không gây xung đột logic va chạm (Collision) đã viết.

---

### II. MÔ HÌNH THUẬT TOÁN (THE MATH & LOGIC MODEL)

Hệ thống tính toán dựa trên một chu kỳ thời gian tổng ($progress$) được chuẩn hóa từ $0.0$ đến $1.0$.

#### 1. Phân mảnh Timeline (Timeline Splitting)
Để tránh hiện tượng nhân vật vừa nhún đà vừa bị "trượt" trên mặt đất, chu kỳ nhảy được chia làm hai biến số:
* **Thời gian Hoạt hình ($progress$):** Chạy từ $0.0 \rightarrow 1.0$, dùng để tính toán mức độ co giãn (Squash & Stretch) cho 4 pha: Lấy đà, Bay, Chạm đất, Hồi phục.
* **Thời gian Bay ($air\_t$):** Bị khóa cứng ở $0.0$ trong pha lấy đà ($progress < 0.15$), biến thiên lên $1.0$ trong lúc bay, và khóa ở $1.0$ khi chạm đất ($progress > 0.85$).

#### 2. Thuật toán Trục Mặt Đất (X, Y)
Thay thế Linear Lerp bằng hàm **Ease-Out Quad** để tạo lực vọt bùng nổ:
* $$t_{\text{move}} = 1 - (1 - air\_t)^2$$
* Tọa độ tịnh tiến ($ground\_x, ground\_y$) được tính dựa trên $t_{\text{move}}$.

#### 3. Thuật toán Trục Độ Cao (Z)
Sử dụng **Sóng Half-Sine** để tạo đỉnh quỹ đạo lơ lửng và mượt mà:
* $$Z = H_{\text{max}} \cdot \sin(\pi \cdot air\_t)$$

---

### III. QUY TRÌNH SỬA ĐỔI MÃ NGUỒN CHI TIẾT

#### Bước 1: Điều chỉnh Header Lớp People (`include/entities/people.h`)
* **Khai báo Bóng đổ (Shadow):** Khởi tạo một đối tượng hình elip (ví dụ: `sf::CircleShape` nén Y) đại diện cho cái bóng dưới chân nhân vật.
* **Ghi đè Quy trình Vẽ:** Khai báo lại phương thức `render` để làm chủ thứ tự vẽ lớp đồ họa.

#### Bước 2: Khởi tạo Dữ liệu (`src/entities/people.cpp` - Constructor)
* **Định hình Bóng đổ:** Đặt kích thước elip tương xứng với độ rộng khung hình, tô màu đen trong suốt và căn tâm (Origin) nằm vào chính giữa hình elip.
* **Tối ưu Thời lượng:** Đặt `anim_duration` xuống khoảng cực ngắn (từ $0.12\text{s}$ đến $0.15\text{s}$) để thao tác điều khiển đạt độ nhạy bén cao nhất.

#### Bước 3: Nâng cấp Logic Cập nhật (Hàm `updateState`)
Đây là khu vực triển khai thuật toán tính toán ở **Phần II**. Quá trình xử lý mỗi khung hình (Frame) diễn ra theo 6 bước:

1. **Chốt chặn Hitbox:** Cập nhật vị trí Hitbox đứng yên tại điểm xuất phát (khóa va chạm trên không trung) hoặc chuyển sang vị trí đích khi ở đỉnh cú nhảy.
2. **Tính toán Không gian:** Áp dụng công thức tính $ground\_x$, $ground\_y$ và $Z$.
3. **Hợp nhất Tỷ lệ (Scale Stacking):** Tỷ lệ gốc (Base) $\times$ Tỷ lệ co giãn (Squash/Stretch).
4. **Bù trừ Điểm Neo (Anchor Offset):**
   * Đo lường độ chênh lệch kích thước (Delta) giữa hình ảnh gốc và hình ảnh sau khi Scale.
   * Tính tọa độ Render thực tế: Đẩy ảnh sang trái một nửa độ chênh lệch ngang (giữ tâm $X$), và đẩy ảnh lên trên để mép dưới vừa khít $ground\_y$, sau đó tiếp tục trừ đi độ cao $Z$.
5. **Cập nhật Bóng đổ:** Neo tọa độ bóng luôn dính vào mặt đất ($ground\_x, ground\_y$). Tính toán giảm Scale và giảm kênh màu Alpha dựa theo độ lớn của $Z$.
6. **Kết thúc Chu kỳ:** Khôi phục Scale, chốt tọa độ đích và nhả khóa Hitbox về vị trí mới.

#### Bước 4: Sắp xếp Lớp Đồ Họa (Hàm `render`)
* Vẽ đối tượng Bóng đổ ra màn hình trước.
* Vẽ đối tượng Sprite của nhân vật đè lên trên Bóng đổ để tạo chiều sâu 2.5D.
