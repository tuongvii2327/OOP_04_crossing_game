#include "entities/people.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/resource_manager.h"
#include "utils/serialization.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

People::People(Constants::ObjectColor color)
	: DynamicObject(Constants::DynamicObjectType::People, color, nullptr),
	  is_dead(false),
	  step_size(static_cast<float>(Constants::LANE_HEIGHT)),
	  people_color(color),
	  current_dir(Direction::Up),
	  current_anim_state(AnimState::Idle),
	  anim_timer(0.f),
	  anim_duration(0.15f), // Nhảy nhanh & nhạy để triệt tiêu Input Lag
	  is_animating(false)
{
	sprite.setOrigin({0.f, 0.f});

	const Constants::Size people_size = Constants::getSizeInGame(Constants::DynamicObjectType::People);
	const float people_width = static_cast<float>(people_size.width);
	const float people_height = static_cast<float>(people_size.height);

	base_scale_x_ = people_width / static_cast<float>(FRAME_W);
	base_scale_y_ = people_height / static_cast<float>(FRAME_H);
	sprite.setScale(sf::Vector2f(base_scale_x_, base_scale_y_));

	hitbox.size = {people_width * Constants::HITBOX_SIZE_RATIO,
		people_height * Constants::HITBOX_SIZE_RATIO};
	hitbox_offset = {(people_width - hitbox.size.x) / 2.f, (people_height - hitbox.size.y) / 2.f};

	// --- Khởi tạo bóng đổ động (Dynamic Shadow) ---
	// Dùng CircleShape và nén trục Y để tạo hình Elip nằm sát đất
	const float shadow_radius = people_width * 0.40f;
	shadow_.setRadius(shadow_radius);
	shadow_.setOrigin({shadow_radius, shadow_radius});
	shadow_.setFillColor(sf::Color(0, 0, 0, 120));
	shadow_.setScale({1.0f, 0.35f}); // Ép dẹp thành hình Elip

	resetPosition();
}

Constants::DynamicObjectType People::getType() const
{
	return Constants::DynamicObjectType::People;
}

Constants::ObjectColor People::getColor() const
{
	return people_color;
}

void People::updateSpriteRect()
{
	int col = static_cast<int>(current_anim_state);
	int row = static_cast<int>(current_dir);
	int left = col * FRAME_W;
	int top = row * FRAME_H;
	sprite.setTextureRect(sf::IntRect(sf::Vector2i(left, top), sf::Vector2i(FRAME_W, FRAME_H)));
}

void People::updateState(float delta_time)
{
	// Reset cờ just_landed_ mỗi frame để đảm bảo PlayingState bắt trọn vẹn 1 frame duy nhất
	just_landed_ = false;

	if (!is_active)
		return;

	if (is_dead)
	{
		death_anim_timer_ += delta_time;
		float t = std::min(death_anim_timer_ / 0.8f, 1.0f); // 0→1 trong 0.8 giây

		if (death_cause_ == Constants::DeathCause::Drowning)
		{
			// Chìm dần xuống nước: scale thu nhỏ và trượt xuống
			float shrink = 1.0f - t * 0.9f; // 1.0 → 0.1
			sprite.setScale({base_scale_x_ * shrink, base_scale_y_ * shrink});
			sprite.move({0.f, delta_time * 30.f});
			shadow_.setFillColor(sf::Color(0, 0, 0, 0));

			// Làm mờ dần sprite
			uint8_t alpha = static_cast<uint8_t>(255 * (1.0f - t));
			sprite.setColor(sf::Color(255, 255, 255, alpha));
		}
		else if (death_cause_ == Constants::DeathCause::OutOfBounds)
		{
			// Đặt tâm xoay vào giữa frame
			sprite.setOrigin({static_cast<float>(FRAME_W) / 2.f, static_cast<float>(FRAME_H) / 2.f});

			// Xoay tròn 3 vòng đầy đủ (1080 độ)
			sprite.setRotation(sf::degrees(t * 1080.f));

			// HIỆU ỨNG ARCADE POP-UP: Bật nảy vọt lên trên màn hình (-y) trước rồi mới rơi xuống
			// Quỹ đạo ném đứng: vọt lên 250px vào sâu trong tầm nhìn camera rồi rơi xuống đáy
			float bounce_y = -350.f * std::sin(t * static_cast<float>(M_PI)) + (t * t * 300.f);

			const float pw = hitbox.size.x / Constants::HITBOX_SIZE_RATIO;
			const float ph = hitbox.size.y / Constants::HITBOX_SIZE_RATIO;
			sprite.setPosition({position.x + pw / 2.f, position.y + ph / 2.f + bounce_y});

			// Thu nhỏ nhẹ dần và làm mờ dần khi rơi
			float shrink = 1.0f - t * 0.3f;
			sprite.setScale({base_scale_x_ * shrink, base_scale_y_ * shrink});
			uint8_t val = static_cast<uint8_t>(255 * (1.0f - t * 0.4f));
			sprite.setColor(sf::Color(val, val, val, static_cast<uint8_t>(255 * (1.0f - t))));
			shadow_.setFillColor(sf::Color(0, 0, 0, 0));
		}
		// Car: animation đã được xử lý tức thì trong die(), không cần update thêm

		// update particles (bụi tiếp đất hoặc bọt nước)
		for (auto& p : dust_particles_)
		{
			p.lifetime -= delta_time;
			p.shape.move(p.velocity * delta_time);
			float ratio = std::max(0.f, p.lifetime / p.max_lifetime);
			sf::Color col = p.shape.getFillColor();
			p.shape.setFillColor(sf::Color(col.r, col.g, col.b, static_cast<uint8_t>(200 * ratio)));
			const float base_r = 6.f;
			p.shape.setRadius(base_r * ratio);
			p.shape.setOrigin({base_r * ratio / 2.f, base_r * ratio / 2.f});
		}
		dust_particles_.erase(
			std::remove_if(dust_particles_.begin(), dust_particles_.end(),
				[](const Particle& p) { return p.lifetime <= 0.f; }),
			dust_particles_.end());
		return;
	}

	if (is_animating)
	{
		anim_timer += delta_time;
		float progress = std::min(anim_timer / anim_duration, 1.0f);

		// ============================================================
		// BƯỚC 1: HITBOX APEX SNAP
		// Khi đang vọt lên (progress < 0.5): hitbox ở ô xuất phát (tránh
		// kích hoạt va chạm với vật thể ở ô đích khi chưa tới nơi).
		// Khi đang rơi xuống (progress >= 0.5): snap hitbox sang ô đích
		// để tránh chết oan do "bóng ma" hitbox bị tụt lại phía sau.
		// ============================================================
		if (progress < 0.5f)
			hitbox.position = jump_start_pos + hitbox_offset;
		else
			hitbox.position = target_position + hitbox_offset;

		// ============================================================
		// BƯỚC 2: PHÂN MẢNH THỜI GIAN BAY (air_t)
		// air_t = 0 khi progress < 0.15 (đang lấy đà, đứng yên trên đất)
		// air_t = [0,1] khi progress trong [0.15, 0.85] (đang bay)
		// air_t = 1 khi progress > 0.85 (đã hạ cánh, đứng yên trên đất)
		// ============================================================
		float air_t = std::clamp((progress - 0.15f) / 0.70f, 0.0f, 1.0f);

		// Tịnh tiến mặt đất (Ease-Out Quad): vọt nhanh lúc đầu, hãm nhẹ khi gần đích
		float t_move    = 1.0f - (1.0f - air_t) * (1.0f - air_t);
		float ground_x  = jump_start_pos.x + (jump_end_pos.x - jump_start_pos.x) * t_move;
		float ground_y  = jump_start_pos.y + (jump_end_pos.y - jump_start_pos.y) * t_move;

		// Chiều cao không trung (Half-Sine): tạo quỹ đạo Parabol nảy lên rồi rơi xuống
		float z_height  = JUMP_ARC_HEIGHT * std::sin(static_cast<float>(M_PI) * air_t);

		// ============================================================
		// BƯỚC 3: HỢP NHẤT TỶ LỆ (SQUASH & STRETCH)
		// Đồng bộ tuyệt đối với mốc thời gian bay (air_t):
		//   progress < 0.15 : Lấy đà  -> Nén người xuống
		//   progress < 0.85 : Bay     -> Vươn dài người (khi air_t chạy)
		//   progress < 0.95 : Chạm đất-> Dẹp người (tiêu tán lực va đập)
		//   progress >= 0.95: Phục hồi-> Trở về hình dáng gốc
		// ============================================================
		float scale_x, scale_y;
		if (progress < 0.15f)
		{
			current_anim_state = AnimState::Prep;
			float t = progress / 0.15f;
			float squash = std::sin(t * static_cast<float>(M_PI) * 0.5f);
			scale_x = base_scale_x_ * (1.f + 0.15f * squash); // phình ngang
			scale_y = base_scale_y_ * (1.f - 0.18f * squash); // nén dọc
		}
		else if (progress < 0.85f)
		{
			current_anim_state = AnimState::Jump;
			float t = (progress - 0.15f) / 0.70f;
			float stretch = std::sin(t * static_cast<float>(M_PI));
			scale_x = base_scale_x_ * (1.f - 0.10f * stretch); // thon ngang
			scale_y = base_scale_y_ * (1.f + 0.12f * stretch); // vươn dài
		}
		else
		{
			current_anim_state = AnimState::Idle;
			float t = (progress - 0.85f) / 0.15f;
			scale_x = base_scale_x_ * (1.f + 0.18f * (1.f - t)); // dẹp ngang
			scale_y = base_scale_y_ * (1.f - 0.22f * (1.f - t)); // nén dọc
		}
		sprite.setScale(sf::Vector2f(scale_x, scale_y));

		// ============================================================
		// BƯỚC 4: BÙ TRỪ ĐIỂM NEO RENDER (ANCHOR OFFSET)
		// Origin bị khóa tại (0,0) để bảo toàn Hitbox. Khi Scale thay đổi,
		// phải bù trừ để gầm chân luôn bám chặt tại ground_y.
		// Đồng thời trừ z_height để nhân vật nảy lên phía trên màn hình.
		// ============================================================
		float delta_w   = static_cast<float>(FRAME_W) * (scale_x - base_scale_x_);
		float delta_h   = static_cast<float>(FRAME_H) * (scale_y - base_scale_y_);
		float render_x  = ground_x - delta_w / 2.0f;
		float render_y  = ground_y - delta_h - z_height;
		sprite.setPosition(sf::Vector2f(render_x, render_y));
		sprite.setOrigin({0.f, 0.f});
		sprite.setRotation(sf::degrees(0.f));

		// ============================================================
		// BƯỚC 5: CẬP NHẬT BÓNG ĐỔ ĐỘNG
		// Bóng luôn neo ở mặt đất (ground_x, ground_y).
		// Co nhỏ và mờ dần khi z_height tăng (nhân vật lên cao).
		// ============================================================
		float shadow_scale   = 1.0f / (1.0f + 0.008f * z_height);
		uint8_t shadow_alpha = static_cast<uint8_t>(std::max(30.f, 120.f - z_height * 1.2f));
		shadow_.setScale({shadow_scale, shadow_scale * 0.35f});
		shadow_.setFillColor(sf::Color(0, 0, 0, shadow_alpha));
		// Đặt tâm bóng ngay dưới tâm gầm chân nhân vật
		const float people_width = hitbox.size.x / Constants::HITBOX_SIZE_RATIO;
		shadow_.setPosition({ground_x + people_width / 2.0f, ground_y + hitbox.size.y * 1.0f});

		// Bắt sự kiện chạm đất: ngay khi progress vượt 0.85 lần đầu tiên trong cú nhảy.
		// Dùng cờ này đảm bảo không bị miss (bỏ lỡ nhịp) khi framerate giật lùi.
		if (progress >= 0.85f && !has_landed_this_jump_)
		{
			has_landed_this_jump_ = true;
			just_landed_ = true;
			spawnDust({ground_x, ground_y});
		}

		updateSpriteRect();

		// ============================================================
		// BƯỚC 6: KẾT THÚC CHU KỲ
		// ============================================================
		if (progress >= 1.0f)
		{
			is_animating = false;
			current_anim_state = AnimState::Idle;
			position = target_position;
			start_lerp_position = position;
			sprite.setOrigin({0.f, 0.f});
			sprite.setRotation(sf::degrees(0.f));
			sprite.setPosition(position);
			hitbox.position = position + hitbox_offset;
			sprite.setScale(sf::Vector2f(base_scale_x_, base_scale_y_));
			// Bóng đổ tĩnh khi đứng yên: full scale, alpha nhẹ
			shadow_.setScale({1.0f, 0.35f});
			shadow_.setFillColor(sf::Color(0, 0, 0, 90));
			const float pw = hitbox.size.x / Constants::HITBOX_SIZE_RATIO;
			shadow_.setPosition({position.x + pw / 2.0f, position.y + hitbox.size.y * 1.0f});
			updateSpriteRect();
		}
	}
	else
	{
		// Khi KHÔNG đang nhảy (Idle/Standing): luôn sync bóng dưới chân
		shadow_.setScale({1.0f, 0.35f});
		shadow_.setFillColor(sf::Color(0, 0, 0, 90));
		const float pw = hitbox.size.x / Constants::HITBOX_SIZE_RATIO;
		shadow_.setPosition({position.x + pw / 2.0f, position.y + hitbox.size.y * 1.0f});
	}

	// ============================================================
	// CẬP NHẬT HẠT BỤI (PARTICLE UPDATE)
	// Đưa ra ngoài if(is_animating) để bụi tiếp tục rơi & mờ dần ngay
	// cả khi nhân vật đã đứng yên (Idle).
	// ============================================================
	for (auto& p : dust_particles_)
	{
		p.lifetime -= delta_time;
		p.shape.move(p.velocity * delta_time);

		// Mờ dần tuyến tính theo tỉ lệ thời gian còn lại
		float ratio = std::max(0.f, p.lifetime / p.max_lifetime);
		uint8_t alpha = static_cast<uint8_t>(200.f * ratio);
		auto col = p.shape.getFillColor();
		p.shape.setFillColor(sf::Color(col.r, col.g, col.b, alpha));

		// Thu nhỏ dần cùng tốc độ với mờ đi
		const float base_r = 6.f;
		p.shape.setRadius(base_r * ratio);
		p.shape.setOrigin({base_r * ratio / 2.f, base_r * ratio / 2.f});
	}
	dust_particles_.erase(
		std::remove_if(dust_particles_.begin(), dust_particles_.end(),
			[](const Particle& p) { return p.lifetime <= 0.f; }),
		dust_particles_.end()
	);
}

bool People::isColliding(const sf::FloatRect &other_hitbox) const
{
	if (is_dead || !is_active)
		return false;
	return hitbox.findIntersection(other_hitbox).has_value();
}

bool People::tryToMove(float dx, float dy, float camera_top_y, const Map &map)
{
	if (is_dead)
		return false;

	// Block input khi đang trong animation để đảm bảo mỗi bước nhảy hoàn chỉnh
	if (is_animating)
		return false;

	// Kiểm tra biên dựa trên target_position (vị trí logic hiện tại)
	float future_left = target_position.x + hitbox_offset.x + dx;
	float future_right = future_left + hitbox.size.x;
	float future_top = target_position.y + hitbox_offset.y + dy;
	float future_bottom = future_top + hitbox.size.y;

	float left_bound = static_cast<float>(Constants::GAME_PLAY_BAR_WIDTH); // left (140)
	float right_bound = static_cast<float>(Constants::WINDOW_WIDTH);       // right (1400)
	float cam_bottom = camera_top_y + static_cast<float>(Constants::WINDOW_HEIGHT);

	// Kiểm tra biên màn hình
	if (future_left < left_bound || future_right > right_bound || future_top < camera_top_y ||
		future_bottom > cam_bottom)
	{
		return false;
	}

	// Kiểm tra va chạm với ô blocked trong map
	sf::FloatRect future_hitbox_rect({future_left, future_top}, {hitbox.size.x, hitbox.size.y});
	if (map.checkBlockedCollision(future_hitbox_rect))
		return false;

	// Lưu điểm xuất phát (vị trí visual hiện tại) và cập nhật điểm đích logic
	// Không gọi moveBy() vì sprite/hitbox được điều khiển hoàn toàn bởi lerp
	// trong updateState()
	start_lerp_position = position;
	target_position = target_position + sf::Vector2f(dx, dy);

	// Sync jump arc positions to match the new lerp positions
	jump_start_pos = position;
	jump_end_pos = target_position;

	Audio().playSFX(AudioSystem::SFX::Jump);

	is_animating = true;
	anim_timer = 0.f;
	has_landed_this_jump_ = false; // Bắt đầu cú nhảy mới, reset cờ theo dõi
	current_anim_state = AnimState::Prep;
	updateSpriteRect();

	return true;
}

void People::moveBy(sf::Vector2f offset)
{
	DynamicObject::moveBy(offset);
	target_position += offset;
	start_lerp_position += offset;
	jump_start_pos += offset;
	jump_end_pos += offset;
}

void People::die(Constants::DeathCause cause)
{
	is_dead = true;
	death_cause_ = cause;
	death_anim_timer_ = 0.f;

	if (cause == Constants::DeathCause::Car)
	{
		// Bẹp dúm tức thì (squash)
		sprite.setScale({base_scale_x_ * 1.5f, base_scale_y_ * 0.2f});
		sprite.move({0.f, hitbox.size.y * 0.8f});
		sprite.setColor(sf::Color(255, 100, 100, 255));
		shadow_.setFillColor(sf::Color(0, 0, 0, 0));
	}
	else if (cause == Constants::DeathCause::Drowning)
	{
		// Tạo bọt nước xung quanh vị trí chìm
		const float pw = hitbox.size.x / Constants::HITBOX_SIZE_RATIO;
		for (int i = 0; i < 8; ++i)
		{
			Particle p;
			const float base_r = 5.f;
			p.shape.setRadius(base_r);
			p.shape.setOrigin({base_r / 2.f, base_r / 2.f});
			p.shape.setFillColor(sf::Color(150, 200, 255, 200)); // Màu xanh nước nhạt

			float offset_x = (static_cast<float>(std::rand() % 100) / 100.f - 0.5f) * pw;
			p.shape.setPosition({position.x + pw / 2.f + offset_x,
								position.y + hitbox.size.y * 0.7f});

			const float speed_x = (static_cast<float>(std::rand() % 80) + 20.f) *
								  (i % 2 == 0 ? -1.f : 1.f);
			const float speed_y = -(static_cast<float>(std::rand() % 50) + 20.f); // Bắt lên trên
			p.velocity = {speed_x, speed_y};
			p.lifetime = 0.5f + static_cast<float>(std::rand() % 30) / 100.f;
			p.max_lifetime = p.lifetime;

			dust_particles_.push_back(std::move(p));
		}
	}
}

bool People::isDead() const
{
	return is_dead;
}

sf::Vector2f People::getStartPosition(int init_cell, int init_lane) const
{
	const Constants::Size people_size =
		Constants::getSizeInGame(Constants::DynamicObjectType::People);
	float start_x = static_cast<float>(Constants::WINDOW_WIDTH) -
					static_cast<float>(Constants::NUMBER_OF_CELLS - init_cell + 1) *
						static_cast<float>(Constants::CELL_SIZE) +
					(static_cast<float>(Constants::CELL_SIZE) -
						static_cast<float>(people_size.width)) /
						2.f;
	float start_y = (static_cast<float>(Constants::WINDOW_HEIGHT) -
						static_cast<float>(Constants::NUMBER_OF_LANES - init_lane + 1) *
							static_cast<float>(Constants::LANE_HEIGHT)) +
					(static_cast<float>(Constants::LANE_HEIGHT) -
						static_cast<float>(people_size.height)) /
						2.f;
	return sf::Vector2f(start_x, start_y);
}

void People::resetPosition()
{
	position = getStartPosition();
	sprite.setPosition(position);
	hitbox.position = position + hitbox_offset;

	jump_start_pos = position;
	jump_end_pos = position;

	// Sync logic positions
	target_position = position;
	start_lerp_position = position;

	is_dead = false;
	death_cause_ = Constants::DeathCause::None;
	death_anim_timer_ = 0.f;
	is_active = true;

	// Khôi phục scale gốc (recalculate từ constants)
	const Constants::Size people_size = Constants::getSizeInGame(Constants::DynamicObjectType::People);
	base_scale_x_ = static_cast<float>(people_size.width) / static_cast<float>(FRAME_W);
	base_scale_y_ = static_cast<float>(people_size.height) / static_cast<float>(FRAME_H);

	current_anim_state = AnimState::Idle;
	current_dir = Direction::Up;
	is_animating = false;
	sprite.setOrigin({0.f, 0.f});
	sprite.setRotation(sf::degrees(0.f));
	sprite.setScale(sf::Vector2f(base_scale_x_, base_scale_y_));
	sprite.setColor(sf::Color::White); // Reset màu sprite về trắng gốc
	updateSpriteRect();
}

void People::snapTargetX(float new_x)
{
	target_position.x = new_x;
	jump_end_pos.x = new_x;
}

sf::FloatRect People::getBounds() const
{
	return sprite.getGlobalBounds();
}

bool People::handleEvent(const sf::Event &event, float camera_top_y, const Map &map)
{
	if (is_dead || !is_active)
		return false;

	const auto* key = event.getIf<sf::Event::KeyPressed>();
	if (!key)
		return false;

	Direction new_dir = current_dir;
	float dx = 0.f, dy = 0.f;
	if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up)
	{
		new_dir = Direction::Up;
		dy = -step_size;
	}
	else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down)
	{
		new_dir = Direction::Down;
		dy = step_size;
	}
	else if (key->code == sf::Keyboard::Key::A || key->code == sf::Keyboard::Key::Left)
	{
		new_dir = Direction::Left;
		dx = -step_size;
	}
	else if (key->code == sf::Keyboard::Key::D || key->code == sf::Keyboard::Key::Right)
	{
		new_dir = Direction::Right;
		dx = step_size;
	}
	else
	{
		return false;
	}

	Direction prev_dir = current_dir;
	if (new_dir != current_dir)
		current_dir = new_dir;

	// Chỉ giữ hướng mới nếu cú nhảy thực sự bắt đầu. Nếu phím bị chặn
	// (đang trong animation / chạm biên / va chạm) thì hoàn trả hướng cũ,
	// tránh hiện tượng nhân vật quay hướng khi nhảy không thành công.
	if (tryToMove(dx, dy, camera_top_y, map))
		return true;

	current_dir = prev_dir;
	return false;
}

void People::serialize(std::ostream &os) const
{
	Serialization::write(os, is_dead);
	Serialization::write(os, position.x);
	Serialization::write(os, position.y);
	Serialization::write(os, step_size);
	Serialization::write(os, is_active);

	int dir_int = static_cast<int>(current_dir);
	Serialization::write(os, dir_int);
}

void People::deserialize(std::istream &is)
{
	Serialization::read(is, is_dead);
	Serialization::read(is, position.x);
	Serialization::read(is, position.y);
	Serialization::read(is, step_size);
	Serialization::read(is, is_active);

	int dir_int = 0;
	Serialization::read(is, dir_int);
	current_dir = static_cast<Direction>(dir_int);

	is_dead = false;
	is_active = true;
	is_animating = false;
	current_anim_state = AnimState::Idle;
	sprite.setOrigin({0.f, 0.f});
	sprite.setRotation(sf::degrees(0.f));

	updateSpriteRect();
	sprite.setPosition(position);
	hitbox.position = position + hitbox_offset;
	// Đồng bộ target & start sau khi load để không có Lerp ghost
	target_position = position;
	start_lerp_position = position;
}

void People::spawnDust(sf::Vector2f ground_pos)
{
	// Tạo 6 hạt bụi văng ra hai bên từ gầm chân nhân vật.
	// Mỗi hạt bay theo hướng ngang khác nhau và tản nhẹ xuống dưới (chuẩn 3/4 View).
	const int   num_particles = 6;
	const float max_lifetime  = 0.28f;
	const float people_w      = hitbox.size.x / Constants::HITBOX_SIZE_RATIO;

	// Dùng random engine đơn giản vì chỉ dùng 1 lần mỗi cú nhảy
	for (int i = 0; i < num_particles; ++i)
	{
		Particle p;
		const float base_r = 6.f;
		p.shape.setRadius(base_r);
		p.shape.setOrigin({base_r / 2.f, base_r / 2.f});
		p.shape.setFillColor(sf::Color(210, 195, 165, 200)); // Màu cát/bụi nhạt

		// Tâm đặt ngay dưới gầm chân, phân tán nhẹ theo chiều ngang
		float offset_x = (static_cast<float>(std::rand() % 100) / 100.f - 0.5f) * people_w;
		p.shape.setPosition({ground_pos.x + people_w / 2.f + offset_x,
							ground_pos.y + hitbox.size.y * 0.9f});

		// Vecto vận tốc: văng ngang mạnh, xuống nhẹ (3/4 View: không có trục Z thật)
		const float speed_x = (static_cast<float>(std::rand() % 120) + 40.f) *
							  (i % 2 == 0 ? -1.f : 1.f); // xen kẽ trái/phải
		const float speed_y = static_cast<float>(std::rand() % 40) + 10.f;  // xuống nhẹ
		p.velocity     = {speed_x, speed_y};
		p.lifetime     = max_lifetime * (0.6f + static_cast<float>(std::rand() % 40) / 100.f);
		p.max_lifetime = p.lifetime;

		dust_particles_.push_back(std::move(p));
	}
}

void People::render(sf::RenderWindow &window) const
{
	if (!is_active)
		return;

	// Thứ tự vẽ (dưới -> trên) để tạo chiều sâu 2.5D:
	// 1. Bóng đổ (sát mặt đất - dưới cùng)
	// 2. Bụi tiếp đất (trên mặt đất, bên dưới chân)
	// 3. Sprite nhân vật (trên cùng)
	window.draw(shadow_);
	for (const auto& p : dust_particles_)
		window.draw(p.shape);
	window.draw(sprite);
}