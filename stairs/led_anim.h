#define NUM_STEPS 16
#define LEDS_PER_STEP 27
#define POINTS_PER_STEP (LEDS_PER_STEP / 3) //LOGICAL_LEDS_PER_STEP
#define MAX_POWER_SUPPLY 60 //watts

#include "sensor.h"

CRGB leds[NUM_STEPS * POINTS_PER_STEP];

int FPS = 64;
int FRAME_MS = 1000 / FPS;
int MODES_FOR_RANDOM[] = {1, 2, 3, 4, 5, 10, 11, 12};
int MAX_ILLUM = 255;

const CRGB Black = CRGB(0,0,0);
const CRGB White = CRGB(MAX_ILLUM,MAX_ILLUM,MAX_ILLUM);
const CRGB Red = CRGB(MAX_ILLUM,0,0);
const CRGB Green = CRGB(0,MAX_ILLUM,0);
const CRGB Blue = CRGB(0,0,MAX_ILLUM);
const CRGB Cyan = CRGB(0,MAX_ILLUM,MAX_ILLUM);
const CRGB Magenta = CRGB(MAX_ILLUM,0,MAX_ILLUM);
const CRGB Yellow = CRGB(MAX_ILLUM,MAX_ILLUM,0);
const CRGB Grey = CRGB(MAX_ILLUM/2,MAX_ILLUM/2,MAX_ILLUM/2);
const CRGB DGrey = CRGB(MAX_ILLUM/4,MAX_ILLUM/4,MAX_ILLUM/4);
const CRGB LGrey = CRGB(MAX_ILLUM-MAX_ILLUM/4,MAX_ILLUM-MAX_ILLUM/4,MAX_ILLUM-MAX_ILLUM/4);
const CRGB colors_list[] = {Red,Red,Green,Blue,Cyan,Magenta,Yellow};
const String colors_txt_list[] = {"Red","Red","Green","Blue","Cyan","Magenta","Yellow"};

const float red_watts_per_led = 0.0767;  // 9 Watt per meter (WS2811 30 leds per meter)
const float green_watts_per_led = 0.1116;
const float blue_watts_per_led = 0.1116;

const String work_modes[] = {"OFF", "ALWAYS-ON", "NIGHT-ON", "SENSORS", "MUSIC"};
int work_mode = findInIndex("SENSORS", work_modes);
const String rnd_modes[] = {"RND-OFF", "RND-M", "RND-C", "RND-M-C"};
int rnd_mode = findInIndex("RND-M-C", rnd_modes);
int animation_mode = 12;
int music_mode = 4;
CRGB main_color1 = Red;
CRGB main_color2 = Blue;
String main_color1_txt = "Red";
String main_color2_txt = "Blue";

int UP = 1;
int DOWN = -1;
int direction = UP;
int last_direction = 0;
bool is_start_animation = false;
int animation_frame = 0;
int max_animation_frame = 0;
int key_frames = 1;
int key_frame = 0;
float progress = 0;
float degress = 1.0 -progress;
const int center_num = POINTS_PER_STEP / 2;
int step_num = 0; // 0..NUM_STEPS-1
int point_num = 0;// 0..POINTS_PER_STEP-1
int step_on;
int step_on_prev1 = -1;
int step_on_prev2 = -1;
int first_step;
int last_step;
CRGB pal_color1 = Red;
CRGB pal_color2 = Green;


int sine_i;
#define SINE_MASK_ROWS 16
#define SINE_MASK_COLS 9
CRGB sine_mask[SINE_MASK_ROWS][SINE_MASK_COLS] = {
  {DGrey, DGrey, DGrey, LGrey, White, LGrey, DGrey, DGrey, DGrey},
  {DGrey, DGrey, DGrey, DGrey, DGrey, Grey, White, Grey, DGrey},
  {DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, Grey, White, DGrey},
  {DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, Grey, LGrey},
  {DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, White},
  {DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, Grey, LGrey},
  {DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, Grey, White, DGrey},
  {DGrey, DGrey, DGrey, DGrey, DGrey, Grey, White, Grey, DGrey},
  {DGrey, DGrey, DGrey, LGrey, White, LGrey, DGrey, DGrey, DGrey},
  {DGrey, Grey, White, Grey, DGrey, DGrey, DGrey, DGrey, DGrey},
  {DGrey, White, Grey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey},
  {LGrey, Grey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey},
  {White, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey},
  {LGrey, Grey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey},
  {DGrey, White, Grey, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey},
  {DGrey, Grey, White, DGrey, DGrey, DGrey, DGrey, DGrey, DGrey}
};
CRGB arrow_mask[SINE_MASK_ROWS][SINE_MASK_COLS] = {
    {DGrey,DGrey,DGrey,DGrey,Grey,DGrey,DGrey,DGrey,DGrey},
    {DGrey,DGrey,DGrey,Grey,White,Grey,DGrey,DGrey,DGrey},
    {DGrey,DGrey,Grey,White,Grey,White,Grey,DGrey,DGrey},
    {DGrey,Grey,White,Grey,DGrey,Grey,White,Grey,DGrey},
    {Grey,White,Grey,DGrey,DGrey,DGrey,Grey,White,Grey},
    {White,Grey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey,White},
    {Grey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey},
    {DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey},
    {DGrey,DGrey,DGrey,DGrey,Grey,DGrey,DGrey,DGrey,DGrey},
    {DGrey,DGrey,DGrey,Grey,White,Grey,DGrey,DGrey,DGrey},
    {DGrey,DGrey,Grey,White,Grey,White,Grey,DGrey,DGrey},
    {DGrey,Grey,White,Grey,DGrey,Grey,White,Grey,DGrey},
    {Grey,White,Grey,DGrey,DGrey,DGrey,Grey,White,Grey},
    {White,Grey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey,White},
    {Grey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey},
    {DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey}
};

/*
bool isModeForMusic(int mode) {
    int modesCount = sizeof(MODES_FOR_MUSIC) / sizeof(MODES_FOR_MUSIC[0]);
    for (int i = 0; i < modesCount; i++) {
        if (MODES_FOR_MUSIC[i] == mode) {
            return true;
        }
    }
    return false;
}*/

float calc_power() {
  float power = 0;
  int led_num;
  for (int step_i = 0; step_i < NUM_STEPS; step_i++) {
    led_num = step_i * POINTS_PER_STEP;
    for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
      power = power + red_watts_per_led * leds[led_num + led_i].r / 255;
      power = power + green_watts_per_led * leds[led_num + led_i].g / 255;
      power = power + blue_watts_per_led * leds[led_num + led_i].b / 255;
    }
  }
  return power;
}
void scale_power(float scale) {
  int led_num;
  for (int step_i = 0; step_i < NUM_STEPS; step_i++) {
    led_num = step_i * POINTS_PER_STEP;
    for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
      leds[led_num + led_i].r = leds[led_num + led_i].r * scale;
      leds[led_num + led_i].g = leds[led_num + led_i].g * scale;
      leds[led_num + led_i].b = leds[led_num + led_i].b * scale;
    }
  }
}

void set_direction(int new_direction) {
  direction = new_direction;
  //Serial.println("direction: " + String(direction));
  //log("set direction " + String(direction));
  //log_matrix(String(direction));
}

int calc_distance(int step1, int point1, int step2, int point2) {
    return abs(point2 - point1) + abs(step2 - step1) / 4;
}

bool check_frames(int frame_from, int frame_to, int key_frame_interval) {
    if (max_animation_frame < frame_to) {
        max_animation_frame = frame_to;
    }

    if (animation_frame >= frame_from && animation_frame <= frame_to) {
        if (animation_frame % key_frame_interval == 1 || frame_from == frame_to) {
            key_frames = intdiv(frame_to - frame_from - key_frame_interval, key_frame_interval);
            key_frame = intdiv(animation_frame - frame_from, key_frame_interval); // 0..key_frames-1

            if (key_frames < 1) {
                key_frames = 1;
            }

            progress = (float)key_frame / (float)key_frames;

            if (progress < 0.03) progress = 0.0;
            if (progress > 1.0)  progress = 1.0;
            degress = 1.0 - progress;
            
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void copy_step(int step_from, int step_to) {
    for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
        leds[step_to * POINTS_PER_STEP + led_i] = leds[step_from * POINTS_PER_STEP + led_i];
    }
}
void copy_point(int step, int point, int from_step, int from_point) {
    if (step >= 0 && step < NUM_STEPS && point >= 0 && point < POINTS_PER_STEP) {
        if (from_step >= 0 && from_step < NUM_STEPS && from_point >= 0 && from_point < POINTS_PER_STEP) {
            leds[step * POINTS_PER_STEP + point] = leds[from_step * POINTS_PER_STEP + from_point];
        }
    }
}
void shift_step(int step, int shift_points) {
    if (shift_points >= POINTS_PER_STEP || shift_points <= -POINTS_PER_STEP) {
        shift_points = shift_points % POINTS_PER_STEP;
    }

    CRGB prev_leds[POINTS_PER_STEP];
    for (int i = 0; i < POINTS_PER_STEP; i++) {
        prev_leds[i] = leds[step * POINTS_PER_STEP + i];
    }

    int led_dest;
    for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
        led_dest = led_i + shift_points;
        if (led_dest < 0) {
            led_dest += POINTS_PER_STEP;
        } else if (led_dest >= POINTS_PER_STEP) {
            led_dest -= POINTS_PER_STEP;
        }
        leds[step * POINTS_PER_STEP + led_dest] = prev_leds[led_i];
    }
}

void draw_step(int step, CRGB points[]) {
    if (step >= 0 && step < NUM_STEPS) {
        for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            if (points[led_i]) {
                leds[step * POINTS_PER_STEP + led_i] = points[led_i];
            } else {
                leds[step * POINTS_PER_STEP + led_i] = CRGB::Black;
            }
        }
    }
}

void illum_step(int step, int illum) {
    int led_num;
    if (step >= 0 && step < NUM_STEPS) {
        for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            led_num = step * POINTS_PER_STEP + led_i;
            leds[led_num].r = constrain(leds[led_num].r + illum, 0, 255);
            leds[led_num].g = constrain(leds[led_num].g + illum, 0, 255);
            leds[led_num].b = constrain(leds[led_num].b + illum, 0, 255);
        }
    }
}

void fill_step(int step, CRGB color) {
    if (step >= 0 && step < NUM_STEPS) {
        for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            leds[step * POINTS_PER_STEP + led_i] = color;
        }
    }
}

void fill_point(int step, int point, CRGB color) {
    if (step >= 0 && step < NUM_STEPS) {
        if (point >= 0 && point < POINTS_PER_STEP) {
            leds[step * POINTS_PER_STEP + point] = color;
        }
    }
}

void draw_step2(int step, CRGB color1, CRGB color2) {
    if (step >= 0 && step < NUM_STEPS) {
        int max_num = POINTS_PER_STEP - 1;
        for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            leds[step * POINTS_PER_STEP + led_i] = CRGB(
                (color1.r * (max_num - led_i) + color2.r * led_i) / max_num,
                (color1.g * (max_num - led_i) + color2.g * led_i) / max_num,
                (color1.b * (max_num - led_i) + color2.b * led_i) / max_num
            );
        }
    }
}

void draw_step3(int step, CRGB color1, CRGB color2, CRGB color3) {
    int max_num = center_num;
    if (step >= 0 && step < NUM_STEPS) {
        for (int led_i = 0; led_i < center_num; led_i++) {
            leds[step * POINTS_PER_STEP + led_i] = CRGB(
                (color1.r * (max_num - led_i) + color2.r * led_i) / max_num,
                (color1.g * (max_num - led_i) + color2.g * led_i) / max_num,
                (color1.b * (max_num - led_i) + color2.b * led_i) / max_num
            );
        }
        for (int led_i = center_num; led_i < POINTS_PER_STEP; led_i++) {
            leds[step * POINTS_PER_STEP + led_i] = CRGB(
                (color2.r * (max_num - (led_i - center_num)) + color3.r * (led_i - center_num)) / max_num,
                (color2.g * (max_num - (led_i - center_num)) + color3.g * (led_i - center_num)) / max_num,
                (color2.b * (max_num - (led_i - center_num)) + color3.b * (led_i - center_num)) / max_num
            );
        }
    }
}

void move_all() {
    if (direction == UP) {
        for (int step_i = NUM_STEPS - 1; step_i > 0; step_i--) {
            copy_step(step_i - 1, step_i);
        }
    } else {
        for (int step_i = 0; step_i < NUM_STEPS - 1; step_i++) {
            copy_step(step_i + 1, step_i);
        }
    }
}

void clear_all() {
    for (int step_i = 0; step_i < NUM_STEPS; step_i++) {
        fill_step(step_i, CRGB::Black);
    }
}

int get_random_color_index() {
    int max_index = sizeof(colors_list) / sizeof(colors_list[0]) - 1;
    int next_index = my_random(1, max_index);
    return next_index;
}

int get_random_mode_index() {
    int max_index = sizeof(MODES_FOR_RANDOM) / sizeof(MODES_FOR_RANDOM[0]) - 1;
    int next_index = my_random(0, max_index);
    //int next_animation_mode = MODES_FOR_RANDOM[next_index];
    return next_index;
}

CRGB calc_back_color() {
    CRGB back_color = CRGB(main_color1.r/8, main_color1.g/8, main_color1.b/8);
    return back_color;
}



void start_animation() {
  is_start_animation = true;

  if (is_start_animation && direction == UP) {
    animation_frame = 1;
    first_step = 0;
    last_step = NUM_STEPS - 1;
    is_start_animation = false;
    last_direction = direction;
    last_start_distance_top = 0;
    last_start_distance_bottom = distance_bottom;
    last_start_voltage_sensor[1] = voltage_pin[1];
    last_start_voltage_sensor[2] = voltage_pin[2];
    last_start_voltage_sensor[3] = 0;
    last_start_voltage_sensor[4] = 0;
    last_start_value_sensor[1] = value_pin[1];
    last_start_value_sensor[2] = value_pin[2];
    last_start_value_sensor[3] = 0;
    last_start_value_sensor[4] = 0;
    log("start_animation UP t:" + String((int)last_start_distance_top) + " b:" + String((int)last_start_distance_bottom));
  }
  if (is_start_animation && direction == DOWN) {
    animation_frame = 1;
    first_step = NUM_STEPS - 1;
    last_step = 0;
    is_start_animation = false;
    last_direction = direction;
    last_start_distance_top = distance_top;
    last_start_distance_bottom = 0;
    last_start_voltage_sensor[1] = 0;
    last_start_voltage_sensor[2] = 0;
    last_start_voltage_sensor[3] = voltage_pin[3];
    last_start_voltage_sensor[4] = voltage_pin[4];
    last_start_value_sensor[1] = 0;
    last_start_value_sensor[2] = 0;
    last_start_value_sensor[3] = value_pin[3];
    last_start_value_sensor[4] = value_pin[4];
    log("start_animation DOWN t:" + String((int)last_start_distance_top) + " b:" + String((int)last_start_distance_bottom));
  }

}

void finish_animation() {
  if (animation_frame > 0) {
    animation_frame = 0;
    clear_all();

    if (rnd_mode == findInIndex("RND-M", rnd_modes) || rnd_mode == findInIndex("RND-M-C", rnd_modes)) {
        //log("next A: " + String(rnd_mode));
        // Смена случайного animation_mode
        int next_index = get_random_mode_index();
        int next_animation_mode = MODES_FOR_RANDOM[next_index];
        //log("next B: " + String(next_index) + "  " + String(next_animation_mode));
        if (next_animation_mode == animation_mode) {
            next_index = get_random_mode_index();
            next_animation_mode = MODES_FOR_RANDOM[next_index];
            //log("next B: " + String(next_index) + "  " + String(next_animation_mode));
            if (next_animation_mode == animation_mode) {
                next_index = get_random_mode_index();
                next_animation_mode = MODES_FOR_RANDOM[next_index];
                //log("next B: " + String(next_index) + "  " + String(next_animation_mode));
            }
        }
        log("next animation_mode: " + String(next_animation_mode));
        animation_mode = next_animation_mode;
    }

    if (rnd_mode == findInIndex("RND-C", rnd_modes) || rnd_mode == findInIndex("RND-M-C", rnd_modes)) {
        // Смена случайного цвета main_color1
        int new_col_index = get_random_color_index();
        CRGB next_main_color1 = colors_list[new_col_index];
        log("next C: " + String(new_col_index) + "  " + rgb_to_str(next_main_color1));
        if (next_main_color1 == main_color1) {
            new_col_index = get_random_color_index();
            next_main_color1 = colors_list[new_col_index];
            if (next_main_color1 == main_color1) {
                new_col_index = get_random_color_index();
                next_main_color1 = colors_list[new_col_index];
            }
        }
        main_color1 = next_main_color1;
        main_color1_txt = colors_txt_list[new_col_index];

        // Смена случайного цвета main_color2
        new_col_index = get_random_color_index();
        main_color2 = colors_list[new_col_index];
        if (main_color2 == main_color1) {
            new_col_index = get_random_color_index();
            main_color2 = colors_list[new_col_index];
            if (main_color2 == main_color1) {
                new_col_index = get_random_color_index();
                main_color2 = colors_list[new_col_index];
            }
        }
        main_color2_txt = colors_txt_list[new_col_index];

        //Serial.print("next colors: ");
        Serial.print("next colors: " + String(main_color1.r) + "." + String(main_color1.g) + "." + String(main_color1.b) + "   ");
        Serial.println(String(main_color2.r) + "." + String(main_color2.g) + "." + String(main_color2.b) );
        Serial.println("---------------------------------------------");
    }
  }
}


void animate_loop() {
  max_animation_frame = 0;

  if (work_mode != 4 /*MUSIC*/) {

    if (animation_mode == 1) {
        //slow gradient wave
        int waves_count = 4;
        int frames_per_wave = 128;
        CRGB wave_color1 = main_color1;
        CRGB wave_color2 = main_color2;

        for (int wave_i=0; wave_i<waves_count; wave_i++) {
            if (check_frames(wave_i*frames_per_wave + 1, wave_i*frames_per_wave + 32, 4)) {
                fill_step(first_step, CRGB(progress*wave_color1.r, progress*wave_color1.g, progress*wave_color1.b));
            }
            if (check_frames(wave_i*frames_per_wave + 32, wave_i*frames_per_wave + 64, 4)) {
                fill_step(first_step, CRGB(degress*wave_color1.r + progress*wave_color2.r,
                                        degress*wave_color1.g + progress*wave_color2.g,
                                        degress*wave_color1.b + progress*wave_color2.b));
            }
            if (check_frames(wave_i*frames_per_wave + 64, wave_i*frames_per_wave + 96, 4)) {
                fill_step(first_step, CRGB(degress*wave_color2.r, degress*wave_color2.g, degress*wave_color2.b));
            }
        }
        if (check_frames(4, (waves_count-1)*frames_per_wave + 128 + 16, 4)) {
            move_all();
        }
    }

    if (animation_mode == 2) {
        //slow ball
        CRGB wave_color1 = main_color1;
        int seconds = 10;
        int rolls_per_sec = 2; // 1,2,4
        int max_balls = 10;
        CRGB back_color = calc_back_color();
        //console.log('frames_for_1step',rolls_per_sec, FRAME_MS, frames_for_1step);

        if (animation_frame == 1) {
            for (int i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        //1st ball
        for (int ball_i=0; ball_i<max_balls; ball_i++) {
            if (check_frames(ball_i * FPS/rolls_per_sec*2 +1, ball_i * FPS/rolls_per_sec*2 +FPS/rolls_per_sec/2 +1,4)) {
                draw_step3(first_step, back_color,
                    CRGB(wave_color1[0]/4 + progress*(wave_color1.r-wave_color1.r/4),
                        wave_color1[1]/4 + progress*(wave_color1.g-wave_color1.g/4),
                        wave_color1[2]/4 + progress*(wave_color1.b-wave_color1.b/4)),
                    back_color);
            }
            if (check_frames(ball_i * FPS/rolls_per_sec*2 +FPS/rolls_per_sec/2 +1, ball_i * FPS/rolls_per_sec*2 +FPS/rolls_per_sec +1,4)) {
                draw_step3(first_step, back_color,
                    CRGB(wave_color1[0]/4 + degress*(wave_color1.r-wave_color1.r/4),
                        wave_color1[1]/4 + degress*(wave_color1.g-wave_color1.g/4),
                        wave_color1[2]/4 + degress*(wave_color1.b-wave_color1.b/4)),
                    back_color);
            }
        }

        if (check_frames(2,(seconds+1) * FPS,4)) {
            move_all();
        }
    }
    

    if (animation_mode == 3) {
        //single color wave
        CRGB wave_color1 = main_color1;
        CRGB back_color = Black; //calc_back_color();
        int speedon = 16;
        int speedoff = 32;

        if (animation_frame == 1) {
            for (int i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        if (direction == UP){
            for (int i = 0; i < NUM_STEPS; i++) {
                if (check_frames(i*speedon,i*speedon+32,4)) {
                    draw_step3(i,
                        back_color,
                        CRGB(progress*wave_color1.r,progress*wave_color1.g,progress*wave_color1.b),
                        back_color);
                }

                if (check_frames(i*speedon+32,i*speedon+64,4)) {
                    draw_step3(i,
                        CRGB(progress*wave_color1.r,progress*wave_color1.g,progress*wave_color1.b),
                        wave_color1,
                        CRGB(progress*wave_color1.r, progress*wave_color1.g, progress*wave_color1.b));
                }
                if (i>0){
                    if (check_frames(i*speedoff+64,i*speedoff+96,4)) {
                        //draw_step3(i-1, Red, CRGB(degress*MAX_ILLUM, 0, 0), Red);
                        draw_step3(i-1,
                            wave_color1,
                            CRGB(degress*wave_color1.r,degress*wave_color1.g,degress*wave_color1.b),
                            wave_color1);
                    }

                    if (check_frames(i*speedoff+96,i*speedoff+128,4)) {
                        draw_step3(i-1,
                            CRGB(degress*wave_color1.r, degress*wave_color1.g, degress*wave_color1.b),
                            back_color,
                            CRGB(degress*wave_color1.r, degress*wave_color1.g, degress*wave_color1.b));
                    }
                }

            }

        } else {
            for (int i = NUM_STEPS-1; i >= 0; i--) {
                if (check_frames((NUM_STEPS-i-1)*speedon,(NUM_STEPS-i-1)*speedon+32,4)) {
                    draw_step3(i,
                        back_color,
                        CRGB(progress*wave_color1.r,progress*wave_color1.g,progress*wave_color1.b),
                        back_color);
                }

                if (check_frames((NUM_STEPS-i-1)*speedon+32,(NUM_STEPS-i-1)*speedon+64,4)) {
                    draw_step3(i,
                        CRGB(progress*wave_color1.r,progress*wave_color1.g,progress*wave_color1.b),
                        wave_color1,
                        CRGB(progress*wave_color1.r,progress*wave_color1.g,progress*wave_color1.b));
                }
                if (i<NUM_STEPS){
                    if (check_frames((NUM_STEPS-i-1)*speedoff+64,(NUM_STEPS-i-1)*speedoff+96,4)) {
                        draw_step3(i+1,
                            wave_color1,
                            CRGB(degress*wave_color1.r, degress*wave_color1.g, degress*wave_color1.b),
                            wave_color1);
                    }

                    if (check_frames((NUM_STEPS-i-1)*speedoff+96,(NUM_STEPS-i-1)*speedoff+128,4)) {
                        draw_step3(i+1,
                            CRGB(degress*wave_color1.r, degress*wave_color1.g, degress*wave_color1.b),
                            back_color,
                            CRGB(degress*wave_color1.r, degress*wave_color1.g, degress*wave_color1.b));
                    }
                }

            }
        }
    }

    if (animation_mode == 4) {
        //fast waves
        CRGB wave_color1 = main_color1;
        int seconds = 12;
        int rolls_per_sec = 2; // 1,2,4
        CRGB back_color = calc_back_color();
        int frames_for_1step = FPS / rolls_per_sec / NUM_STEPS;

        if (animation_frame == 1) {
            for (int i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }
        step_on_prev1 = step_on;
        if (direction == UP) {
            step_on = (animation_frame-1)/frames_for_1step % NUM_STEPS;
        } else {
            step_on = NUM_STEPS-1 - ((animation_frame-1)/frames_for_1step % NUM_STEPS);
        }
        if (step_on != step_on_prev1) {
            if (direction == UP) {
                step_on_prev2 = step_on - 2;
                if (step_on_prev2 < 0) {
                    step_on_prev2 = step_on_prev2 + NUM_STEPS;
                }
            } else {
                step_on_prev2 = step_on + 2;
                if (step_on_prev2 >= NUM_STEPS) {
                    step_on_prev2 = step_on_prev2 - NUM_STEPS;
                }
            }
            //console.log('frames_for_1step',frames_for_1step, animation_frame, [step_on, step_on_prev1, step_on_prev2]);
            fill_step(step_on_prev2, back_color);
            fill_step(step_on_prev1, CRGB(wave_color1.r/1.5, wave_color1.g/1.5, wave_color1.b/1.5));
            fill_step(step_on, wave_color1);
        }
        if (check_frames(1,seconds*FPS,5)) {
            //do nothing, just set max_animation_frame = times*FPS
        }

    }

    if (animation_mode == 5) {
        //worms
        int seconds = 12;
        max_animation_frame = seconds*FPS;
        int frames_per_step = 3;
        int worm_len = 3;
        int worm_max_age = 6 + worm_len-1;
        int worm_count = 2;
        int frames_per_spawn = worm_max_age/2 *frames_per_step;
        CRGB worm_color = main_color1;
        CRGB back_color = calc_back_color();

        if (animation_frame == 1) {
            //worms = [];
            wormManager.clear();
            for (int i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        // spawn
        if (animation_frame % frames_per_spawn == 1) {
            for (int i = 0; i < worm_count; i++) {
                int point;
                int step;
                int loop_counter = 0; 
                int d = 0;
                while (loop_counter <= 50) {
                    point = my_random(0, POINTS_PER_STEP - 1);
                    if (direction == UP) {
                        step = my_random(0, NUM_STEPS - 1 - worm_max_age / 2);
                    } else {
                        step = my_random( worm_max_age / 2, NUM_STEPS - 1);
                    }
                    int d_min = 99;
                    for (int worm_i = 0; worm_i < wormManager.size(); worm_i++) {
                        int age_i = (animation_frame - wormManager.worms[worm_i].animation_frame) / frames_per_step;
                        if (direction == UP) {
                            d = calc_distance(step, point, wormManager.worms[worm_i].step + age_i, wormManager.worms[worm_i].point);
                        } else {
                            d = calc_distance(step, point, wormManager.worms[worm_i].step - age_i, wormManager.worms[worm_i].point);
                        }
                        if (d < d_min) {
                            d_min = d;
                        }
                    }
                    if (d_min >= 2 || wormManager.size() == 0) {
                        break;
                    }
                    loop_counter++;
                }
                //console.log('loop_counter', worms.length, loop_counter);
                //worms.push({step: step, point: point, animation_frame: animation_frame});
                wormManager.add(step, point, animation_frame);
            }
        }

        if (animation_frame % frames_per_step == 1) {
            // die
            for (int worm_i = wormManager.size()-1; worm_i >= 0; worm_i--) {
                int age_i = (animation_frame - wormManager.worms[worm_i].animation_frame) / frames_per_step;
                if (age_i >= worm_max_age) {
                    for (int a = 0; a < worm_len; a++) {
                        if (direction == UP) {
                            fill_point(wormManager.worms[worm_i].step + age_i - a - 1, wormManager.worms[worm_i].point, back_color);
                        } else {
                            fill_point(wormManager.worms[worm_i].step - age_i + a + 1, wormManager.worms[worm_i].point, back_color);
                        }
                    }
                    //worms.splice(worm_i, 1);
                    wormManager.del(worm_i);
                }
            }
            // draw
            for (int worm_i = 0; worm_i < wormManager.size(); worm_i++) {
                int age_i = (animation_frame - wormManager.worms[worm_i].animation_frame) / frames_per_step;
                if (age_i <= worm_max_age - worm_len) {
                    if (direction == UP) {
                        fill_point(wormManager.worms[worm_i].step + age_i, wormManager.worms[worm_i].point, worm_color);
                    } else {
                        fill_point(wormManager.worms[worm_i].step - age_i, wormManager.worms[worm_i].point, worm_color);
                    }
                }
                if (direction == UP) {
                    fill_point(wormManager.worms[worm_i].step + age_i - worm_len, wormManager.worms[worm_i].point, back_color);
                } else {
                    fill_point(wormManager.worms[worm_i].step - age_i + worm_len, wormManager.worms[worm_i].point, back_color);
                }
            }
        }

    }


    if (animation_mode == 10) {
        //snakes
        int frames_per_step = 2;
        int worm_len = POINTS_PER_STEP*5;
        int frames_per_spawn = worm_len * frames_per_step;
        CRGB worm_color = main_color1;
        CRGB back_color = calc_back_color();
        int max_snakes = 8;
        max_animation_frame = (max_snakes+1) * frames_per_spawn;

        if (animation_frame == 1) {
            for (int i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        if (animation_frame % frames_per_step == 1) {
            //spawn
            if (animation_frame <= max_animation_frame - 3*frames_per_spawn) {
                int point = 0;
                int step;
                if (direction == UP) {
                    step = 0;
                } else {
                    step = NUM_STEPS - 1;
                }
                int frames_len = frames_per_spawn - 1 * frames_per_step * POINTS_PER_STEP;
                int cur_len = animation_frame % frames_per_spawn;
                CRGB pal_color_i = CRGB(
                    max((back_color[0] * cur_len / frames_len + worm_color[0] * (frames_len - cur_len) / frames_len), back_color[0]),
                    max((back_color[1] * cur_len / frames_len + worm_color[1] * (frames_len - cur_len) / frames_len), back_color[1]),
                    max((back_color[2] * cur_len / frames_len + worm_color[2] * (frames_len - cur_len) / frames_len), back_color[2])
                );
                fill_point(step, point, pal_color_i);
                //console.log('step', step, point);
            }

            // draw
            if (direction == UP) {
                for (int step_i = NUM_STEPS-1; step_i > 0; step_i -= 2) {
                    shift_step(step_i, -1);
                    copy_point(step_i, POINTS_PER_STEP - 1, step_i-1, POINTS_PER_STEP - 1);
                    shift_step(step_i-1, 1);
                    if (step_i > 1) {
                        copy_point(step_i-1, 0, step_i-2, 0);
                    }
                }
            } else { //DOWN
                for (int step_i = 0; step_i < NUM_STEPS - 1; step_i += 2) {
                    shift_step(step_i, -1);
                    copy_point(step_i, POINTS_PER_STEP - 1, step_i+1, POINTS_PER_STEP - 1);
                    shift_step(step_i+1, 1);
                    if (step_i < NUM_STEPS-2) {
                        copy_point(step_i+1, 0, step_i+2, 0);
                    }
                }
            }

        }
    }

    if (animation_mode == 11) {
        // sine
        CRGB wave_color1 = main_color1;
        const int seconds = 12;
        const int rolls_per_sec = 64; // 1,2,4
        CRGB back_color = calc_back_color();

        if (animation_frame == 1) {
            sine_i = 0;
            for (int i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        if (check_frames(2, seconds * FPS, 4)) {
            move_all();
        }

        if (check_frames(2, seconds * FPS, 4)) {
            // Создаем локальную копию массива цветов
            CRGB points[POINTS_PER_STEP];
            for (int i = 0; i < POINTS_PER_STEP; i++) {
                points[i] = CRGB(
                    (sine_mask[sine_i][i].r * wave_color1.r) / 256,
                    (sine_mask[sine_i][i].g * wave_color1.g) / 256,
                    (sine_mask[sine_i][i].b * wave_color1.b) / 256
                );
            }

            if (direction == UP) {
                draw_step(first_step, points);
            } else {
                draw_step(NUM_STEPS - 1, points);
            }

            sine_i++;
            if (sine_i == SINE_MASK_ROWS) { // Нужно знать длину массива sine_mask
                sine_i = 0;
            }
        }
    }

    if (animation_mode == 12) {
        //arrow
        CRGB arrow_color1 = main_color1;
        const int seconds = 12;
        const int rolls_per_sec = 64; // 1,2,4
        CRGB back_color = calc_back_color();

        if (animation_frame == 1) {
            sine_i = 0;
            for (int i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }
        
        if (check_frames(2, seconds * FPS, 4)) {
            move_all();
        }

        if (check_frames(1, seconds * FPS, 4)) {
            // Создаем локальную копию массива цветов
            CRGB points[POINTS_PER_STEP] = arrow_mask[sine_i];
            for (int i = 0; i < POINTS_PER_STEP; i++) {
                points[i] = CRGB(
                    points[i].r * arrow_color1.r / 256,
                    points[i].g * arrow_color1.g / 256,
                    points[i].b * arrow_color1.b / 256
                );
            }

            if (direction == UP) {
                draw_step(first_step, points);
            } else {
                draw_step(NUM_STEPS - 1, points);
            }

            sine_i++;
            if (sine_i == SINE_MASK_ROWS) { // Нужно знать длину массива sine_mask
                sine_i = 0;
            }
        }
    }

  }

  if (work_mode == 4 /*MUSIC*/) {
    //animation_mode == 6
    if (music_mode == 1) {
        //music vertical single color
        int frames_per_pick_down = 8;
        CRGB main_color = main_color1;
        main_color = CRGB(main_color.r/1.5, main_color.g/1.5, main_color.b/1.5);
        CRGB back_color = Black; //calc_back_color();
        CRGB pic_color = main_color2;
        boolean use_pick = true;

        // Преобразуем значение от 0-100 в диапазон от 0-16
        //int fill_steps = (((float)globalMicValue / 100.0) * NUM_STEPS) - 1;
        int fill_steps = map(globalMicValue, 0, 100, 0, NUM_STEPS);  //NUM_STEPS-1 wrong
        //console.log(fill_steps);

        if (fill_steps >= step_on_prev1) {
            step_on_prev1 = fill_steps;
        } else {
            if (animation_frame % frames_per_pick_down == 1 && step_on_prev1 >= 0) {
                step_on_prev1--;
            }
        }
        //console.log(fill_steps, step_on_prev1);

        for (int i = 0; i < NUM_STEPS; i++) {
            if (i <= fill_steps) {
                fill_step(i, main_color);
            } else {
                fill_step(i, back_color);
            }
            if (use_pick && i == step_on_prev1) {
                fill_step(i, pic_color);
            }
        }
    }

    //animation_mode == 7
    if (music_mode == 2) {
        //music vertical gradient
        if (animation_frame % 2 == 0) {
            CRGB back_color = Black;
            int hue1 = animation_frame % 360; // цикл по кругу оттенков 0–359
            int hue2 = (animation_frame + 270) % 360;
            pal_color1 = hslToCRGB((float)hue1 / 360, 1, 0.5); // s=1, l=0.5 — яркие чистые цвета
            pal_color2 = hslToCRGB((float)hue2 / 360, 1, 0.5);
            //decrease green and blue
            pal_color1 = CRGB(pal_color1.r, pal_color1.g / 4, pal_color1.b / 4);
            pal_color2 = CRGB(pal_color2.r, pal_color2.g / 4, pal_color2.b / 4);

            // Преобразуем значение от 0-100 в диапазон от 0-16
            //int fill_steps = ((globalMicValue / 100) * NUM_STEPS) - 1;
            int fill_steps = map(globalMicValue, 0, 100, 0, NUM_STEPS);
            /*if (animation_frame % 16 == 0) {
                log("colors " + String(hue1) + "/" + String(hue2) + " - " + CRGBtoString(pal_color1) + "/" + CRGBtoString(pal_color2));
            }*/

            for (int step_i = 0; step_i < NUM_STEPS; step_i++) {
                if (step_i <= fill_steps) {
                    CRGB pal_color_i = CRGB(
                        pal_color1.r * step_i / fill_steps + pal_color2.r * (fill_steps - step_i) / fill_steps,
                        pal_color1.g * step_i / fill_steps + pal_color2.g * (fill_steps - step_i) / fill_steps,
                        pal_color1.b * step_i / fill_steps + pal_color2.b * (fill_steps - step_i) / fill_steps
                    );

                    fill_step(step_i, pal_color_i);
                } else {
                    fill_step(step_i, back_color);
                }
            }
        }
    }

    //animation_mode == 8
    if (music_mode == 3) {
        //music horizontal single color
        int frames_per_pick_down = 8;
        CRGB main_color = main_color1;
        main_color = CRGB(main_color.r/1.5, main_color.g/1.5, main_color.b/1.5);
        CRGB back_color = Black; //calc_back_color();
        CRGB pic_color = main_color2;
        boolean use_pick = false;

        // Преобразуем значение от 0-100 в диапазон от 0-9
        //int fill_steps = ((globalMicValue / 100) * POINTS_PER_STEP) - 1;
        int fill_steps = map(globalMicValue, 0, 100, 0, POINTS_PER_STEP);
        //console.log(fill_steps);

        if (fill_steps >= step_on_prev1) {
            step_on_prev1 = fill_steps;
        } else {
            if (animation_frame % frames_per_pick_down == 1 && step_on_prev1 >= 0) {
                step_on_prev1--;
            }
        }

        for (int step_i = 0; step_i < NUM_STEPS; step_i++) {
            for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
                if (led_i <= fill_steps) {
                    leds[step_i * POINTS_PER_STEP + led_i] = main_color;
                } else {
                    leds[step_i * POINTS_PER_STEP + led_i] = back_color;
                }
                if (use_pick && led_i == step_on_prev1) {
                    leds[step_i * POINTS_PER_STEP + led_i] = pic_color;
                }
            }
        }
    }

    //animation_mode == 9
    if (music_mode == 4) {
        //music horizontal gradient
        if (animation_frame % 2 == 0) {
            CRGB back_color = Black;
            int hue1 = animation_frame % 360; // цикл по кругу оттенков 0–359
            int hue2 = (animation_frame + 270) % 360;
            pal_color1 = hslToCRGB((float)hue1 / 360, 1, 0.5); // s=1, l=0.5 — яркие чистые цвета
            pal_color2 = hslToCRGB((float)hue2 / 360, 1, 0.5);
            //decrease green and blue
            pal_color1 = CRGB(pal_color1.r, pal_color1.g / 4, pal_color1.b / 4);
            pal_color2 = CRGB(pal_color2.r, pal_color2.g / 4, pal_color2.b / 4);

            // Преобразуем значение от 0-100 в диапазон от 0-16
            int fill_steps = map(globalMicValue, 0, 100, 0, POINTS_PER_STEP);
            /*if (animation_frame % 16 == 0) {
                log("colors " + String(hue1) + "/" + String(hue2) + " - " + CRGBtoString(pal_color1) + "/" + CRGBtoString(pal_color2));
            }*/

            for (int step_i = 0; step_i < NUM_STEPS; step_i++) {
                for (int led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
                    if (led_i <= fill_steps) {
                        CRGB pal_color_i = CRGB(
                            pal_color1.r * led_i / fill_steps + pal_color2.r * (fill_steps - led_i) / fill_steps,
                            pal_color1.g * led_i / fill_steps + pal_color2.g * (fill_steps - led_i) / fill_steps,
                            pal_color1.b * led_i / fill_steps + pal_color2.b * (fill_steps - led_i) / fill_steps
                        );

                        leds[step_i * POINTS_PER_STEP + led_i] = pal_color_i;
                    } else {
                        leds[step_i * POINTS_PER_STEP + led_i] = back_color;
                    }
                }
            }
        }
    }
  } // if work_mode == MUSIC


    /*float power = calc_power();
    if (power > MAX_POWER_SUPPLY) {
      float scale = (float)MAX_POWER_SUPPLY/power;
      scale_power(scale);
      log("power scaled from:" + String(power));
    }*/

    // check animation finish
    animation_frame++;
    if (animation_frame > max_animation_frame && work_mode != 4/*MUSIC*/) {
        finish_animation();
        if (work_mode == 1 /*ALWAYS-ON*/) {
            direction = -direction;
            start_animation();
        }
    }

}

