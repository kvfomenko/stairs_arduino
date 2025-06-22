const BRIGHTNESS = 255;
const NUM_STEPS = 16;
const LEDS_PER_LOGICAL = 3; //WS2811
const POINTS_PER_STEP = 9; //LOGICAL_LEDS_PER_STEP
const MAX_POWER_SUPPLY = 60;
const FPS = 64;
const FRAME_MS = 1000 / FPS;
const MODES_FOR_RANDOM = [1,2,3,4,5,10,11];
const MODES_FOR_MUSIC = [6,7,8,9];
console.log('FRAME_MS:', FRAME_MS);


const LED_SIZE_W = 20;
const LED_SIZE_H = 40;
const PADDING = 2;
const WIDTH = POINTS_PER_STEP * LEDS_PER_LOGICAL * (LED_SIZE_W + PADDING);
const HEIGHT = NUM_STEPS * (LED_SIZE_H + PADDING);
const HIGH = 1;
const LOW = 1;
const INPUT = 1;
const UP = 1;
const DOWN = -1;
const canvas = document.getElementById("ledCanvas");
const ctx = canvas.getContext("2d");
canvas.width = WIDTH;
canvas.height = HEIGHT;
const SENSOR_TOP = 6;
const SENSOR_BOTTOM = 5;

const EXP = 1.2;                   // степень усиления сигнала (для более "резкой" работы) (по умолчанию 1.4)
const MAX_COEF = 1.8;              // коэффициент громкости (максимальное равно срднему * этот коэф) (по умолчанию 1.8)
const SMOOTH = 0.3;               // коэффициент плавности анимации VU (по умолчанию 0.5)
const LOW_PASS = 80;
let globalMicValue = 0;
document.getElementById('sensor'+SENSOR_TOP).addEventListener("mousedown", () => digitalWrite(SENSOR_TOP, 1));
document.getElementById('sensor'+SENSOR_TOP).addEventListener("mouseup", () => digitalWrite(SENSOR_TOP, 0));
document.getElementById('sensor'+SENSOR_BOTTOM).addEventListener("mousedown", () => digitalWrite(SENSOR_BOTTOM, 1));
document.getElementById('sensor'+SENSOR_BOTTOM).addEventListener("mouseup", () => digitalWrite(SENSOR_BOTTOM, 0));
document.getElementById('work_mode').addEventListener("change", function () {
    work_mode = this.value*1;
    console.log("work_mode:", work_mode);
});
document.getElementById('animation_mode').addEventListener("change", function () {
    animation_mode = this.value*1;
    if (MODES_FOR_MUSIC.includes(animation_mode)) {
        is_start_animation = true;
    }
    console.log("animation_mode:", animation_mode);
});
document.getElementById('main_color1').addEventListener("change", function () {
    main_color1 = colors_list[this.value*1];
    console.log("main_color1:", main_color1);
});
document.getElementById('main_color2').addEventListener("change", function () {
    main_color2 = colors_list[this.value*1];
    console.log("main_color2:", main_color2);
});

let power = 0;
let power_scale = 1;
let pins = [0,0,0,0,0, 0,0,0,0,0, 0,0];
let start_time;
function millis() {
    return Date.now() - start_time;
}
function pinMode(pin, mode) {
}
function intdiv(val, by){
    return (val - val % by) / by;
}
function random(min, max){
    return Math.trunc(Math.random()*(max+1-min)+min)
}
function calc_distance(step1, point1, step2, point2){
    return Math.abs(point2 -point1) + Math.abs(step2 -step1) / 4
}
function digitalWrite(pin, state) {
    pins[pin] = state;
    document.getElementById('pins').innerText = pins.join(" ");
}
function digitalRead(pin) {
    return pins[pin]
}
MAX_ILLUM = 255;
function CRGB(red, green, blue) {
    if (red > MAX_ILLUM) {
        red = MAX_ILLUM
    }
    if (red < 0 || red === undefined || isNaN(red)) {
        red = 0
    }
    if (green > MAX_ILLUM) {
        green = MAX_ILLUM
    }
    if (green < 0 || green === undefined || isNaN(green)) {
        green = 0
    }
    if (blue > MAX_ILLUM) {
        blue = MAX_ILLUM
    }
    if (blue < 0 || blue === undefined || isNaN(blue)) {
        blue = 0
    }
    return [red, green, blue];
}
const Black = CRGB(0,0,0);
const White = CRGB(MAX_ILLUM,MAX_ILLUM,MAX_ILLUM);
const Red = CRGB(MAX_ILLUM,0,0);
const Green = CRGB(0,MAX_ILLUM,0);
const Blue = CRGB(0,0,MAX_ILLUM);
const Cyan = CRGB(0,MAX_ILLUM,MAX_ILLUM);
const Magenta = CRGB(MAX_ILLUM,0,MAX_ILLUM);
const Yellow = CRGB(MAX_ILLUM,MAX_ILLUM,0);
const Grey = CRGB(MAX_ILLUM/2,MAX_ILLUM/2,MAX_ILLUM/2);
const DGrey = CRGB(MAX_ILLUM/4,MAX_ILLUM/4,MAX_ILLUM/4);
const LGrey = CRGB(MAX_ILLUM-MAX_ILLUM/4,MAX_ILLUM-MAX_ILLUM/4,MAX_ILLUM-MAX_ILLUM/4);

const colors_list = [Red,Red,Green,Blue,Cyan,Magenta,Yellow];
const red_watts_per_led = 0.0767;  // 9 Watt per meter (WS2811 30 leds per meter)
const green_watts_per_led = 0.1116;
const blue_watts_per_led = 0.1116;

//let leds = Array(NUM_STEPS * POINTS_PER_STEP).fill(`rgb(0, 0, 0)`);
let leds = Array(NUM_STEPS * POINTS_PER_STEP).fill(Black);

let FastLED = {
    brightness_coef: 1,

    show: function() {
        let color_i;
        //ctx.fillStyle = "grey";
        //ctx.fillRect(0, 0, canvas.width, canvas.height);
        for (let step = 0; step < NUM_STEPS; step++) {
            for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
                color_i = leds[step*POINTS_PER_STEP + led_i];
                if (this.brightness_coef < 1) {
                    color_i = CRGB(color_i[0]*this.brightness_coef, color_i[1]*this.brightness_coef, color_i[2]*this.brightness_coef);
                }
                ctx.fillStyle = `rgb(${color_i[0]},${color_i[1]},${color_i[2]})`;
                ctx.fillRect(led_i * (LED_SIZE_W + PADDING)*3, (NUM_STEPS -1 - step) * (LED_SIZE_H + PADDING), LED_SIZE_W, LED_SIZE_H);
                ctx.fillRect(led_i * (LED_SIZE_W + PADDING)*3 + (LED_SIZE_W + PADDING-1), (NUM_STEPS -1 - step) * (LED_SIZE_H + PADDING), LED_SIZE_W, LED_SIZE_H);
                ctx.fillRect(led_i * (LED_SIZE_W + PADDING)*3 + (LED_SIZE_W + PADDING-1) *2, (NUM_STEPS -1 - step) * (LED_SIZE_H + PADDING), LED_SIZE_W, LED_SIZE_H);
            }
        }
    },
    clear: function() {
        ctx.fillStyle = "grey";
        ctx.fillRect(0, 0, canvas.width, canvas.height);
    },
    setBrightness: function(brightness) {
        this.brightness_coef = brightness / 255;
    }
}

function setupMicrophone() {
    return new Promise((resolve, reject) => {
        navigator.mediaDevices.getUserMedia({ audio: true })
            .then(function(stream) {
                const audioContext = new (window.AudioContext || window.webkitAudioContext)();
                const analyser = audioContext.createAnalyser();
                const microphone = audioContext.createMediaStreamSource(stream);

                microphone.connect(analyser);

                analyser.fftSize = 32;
                const bufferLength = analyser.frequencyBinCount;
                const dataArray = new Uint8Array(bufferLength);

                resolve({ analyser, dataArray, bufferLength });
            })
            .catch(function(err) {
                reject("Error access to mic: " + err);
            });
    });
}

/*function generateBars(number) {
    const hundreds = Math.floor(Math.abs(number) / 10);
    return '|'.repeat(hundreds);
}*/

//fill global variable globalMicValue as 0..100
function getMicrophoneSignalLevel(analyser, dataArray, bufferLength) {
    function getLevel() {
        analyser.getByteFrequencyData(dataArray);
        const MAX_PASS = 200;

        let sum = 0;
        for (let i = 0; i < bufferLength; i++) {
            sum += dataArray[i];
        }

        let micValue = sum / bufferLength;
        micValue = (micValue - LOW_PASS);
        if (micValue < 0) {micValue = 0}

        // возводим в степень (для большей чёткости работы)
        micValue = Math.pow(micValue, EXP);

        micValue = micValue * (100/(Math.pow(MAX_PASS, EXP)-LOW_PASS));

        if (micValue > MAX_PASS) {micValue = MAX_PASS}

        // фильтр сглаживания
        globalMicValue = micValue * SMOOTH + globalMicValue * (1 - SMOOTH);

        globalMicValue = Math.floor(globalMicValue);
        //console.log("globalMicValue: ", generateBars(globalMicValue), globalMicValue);
    }

    setInterval(getLevel, 10);
}

run();

function run() {
    start_time = Date.now();

    setupMicrophone()
        .then(({ analyser, dataArray, bufferLength }) => {
            getMicrophoneSignalLevel(analyser, dataArray, bufferLength);
        })
        .catch(err => {
            console.error(err);
        });

    setup();
    FastLED.show();
    setInterval(loop, FRAME_MS);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
let work_mode = 0;
let animation_mode = 10;
let main_color1 = Red;
let main_color2 = Blue;

let animation_frame = 0;
let max_animation_frame = 0;
let key_frames = 1;
let key_frame = 0;
let progress = 0;  //0..1
let degress = 1-progress; //1..0
let direction = DOWN; //UP;
let last_millis = millis();
let center_num = Math.floor(POINTS_PER_STEP / 2);
let step_num = 0; // 0..NUM_STEPS-1
let point_num = 0;// 0..POINTS_PER_STEP-1
let is_start_animation = false;
let step_on;
let step_on_prev1 = -1;
let step_on_prev2 = -1;
let first_step;
let last_step;
let pal_color1 = Red;
let pal_color2 = Green;
let worms = [];
let sine_i;
let sine_mask = [
    [DGrey,DGrey,DGrey,LGrey,White,LGrey,DGrey,DGrey,DGrey],
    [DGrey,DGrey,DGrey,DGrey,DGrey,Grey,White,Grey,DGrey],
    [DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey,White,DGrey],
    [DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey,LGrey],
    [DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,White],
    [DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey,LGrey],
    [DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey,White,DGrey],
    [DGrey,DGrey,DGrey,DGrey,DGrey,Grey,White,Grey,DGrey],
    [DGrey,DGrey,DGrey,LGrey,White,LGrey,DGrey,DGrey,DGrey],
    [DGrey,Grey,White,Grey,DGrey,DGrey,DGrey,DGrey,DGrey],
    [DGrey,White,Grey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey],
    [LGrey,Grey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey],
    [White,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey],
    [LGrey,Grey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey],
    [DGrey,White,Grey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey],
    [DGrey,Grey,White,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey],
];
let arrow_mask = [
    [DGrey,DGrey,DGrey,DGrey,Grey,DGrey,DGrey,DGrey,DGrey],
    [DGrey,DGrey,DGrey,Grey,White,Grey,DGrey,DGrey,DGrey],
    [DGrey,DGrey,Grey,White,Grey,White,Grey,DGrey,DGrey],
    [DGrey,Grey,White,Grey,DGrey,Grey,White,Grey,DGrey],
    [Grey,White,Grey,DGrey,DGrey,DGrey,Grey,White,Grey],
    [White,Grey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey,White],
    [Grey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey],
    [DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey],
    [DGrey,DGrey,DGrey,DGrey,Grey,DGrey,DGrey,DGrey,DGrey],
    [DGrey,DGrey,DGrey,Grey,White,Grey,DGrey,DGrey,DGrey],
    [DGrey,DGrey,Grey,White,Grey,White,Grey,DGrey,DGrey],
    [DGrey,Grey,White,Grey,DGrey,Grey,White,Grey,DGrey],
    [Grey,White,Grey,DGrey,DGrey,DGrey,Grey,White,Grey],
    [White,Grey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey,White],
    [Grey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,Grey],
    [DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey,DGrey],
];

//console.log('center_num:', center_num);


function setup() {
    /*
    FastLED.addLeds<WS2811, LED_PIN_1, GRB>(leds, 0,                          POINTS_PER_STEP * 4);
    FastLED.addLeds<WS2811, LED_PIN_2, GRB>(leds, POINTS_PER_STEP * 4,  POINTS_PER_STEP * 4);
    FastLED.addLeds<WS2811, LED_PIN_3, GRB>(leds, POINTS_PER_STEP * 8,  POINTS_PER_STEP * 4);
    FastLED.addLeds<WS2811, LED_PIN_4, GRB>(leds, POINTS_PER_STEP * 12, POINTS_PER_STEP * 4);
    */

    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    pinMode(SENSOR_BOTTOM, INPUT);
    pinMode(SENSOR_TOP, INPUT);
}


function loop() {
    if (digitalRead(SENSOR_BOTTOM) == HIGH && animation_frame == 0) {
        direction = UP;
        is_start_animation = true;
    }
    if (digitalRead(SENSOR_TOP) == HIGH && animation_frame == 0) {
        direction = DOWN;
        is_start_animation = true;
    }

    if (is_start_animation && direction == UP) {
        animation_frame = 1;
        first_step = 0;
        last_step = NUM_STEPS - 1;
        is_start_animation = false;
    }
    if (is_start_animation && direction == DOWN) {
        animation_frame = 1;
        first_step = NUM_STEPS - 1;
        last_step = 0;
        is_start_animation = false;
    }

    if (millis() - last_millis >= FRAME_MS-1) {
        last_millis = millis();
        if (animation_frame > 0) {
            animate_loop(animation_frame);
            FastLED.show();
        }
    }
}

function calc_power() {
    let power = 0;
    let led_num;
    //let deb = '';
    for (let step_i = 0; step_i < NUM_STEPS; step_i++) {
        led_num = step_i * POINTS_PER_STEP;
        //deb = deb + 'step_i: ' + step_i + ' = ';
        for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            power = power + red_watts_per_led * leds[led_num + led_i][0] / 255;
            power = power + green_watts_per_led * leds[led_num + led_i][1] / 255;
            power = power + blue_watts_per_led * leds[led_num + led_i][2] / 255;
            //deb = deb + leds[led_num + led_i][0] / 255 + ',' + leds[led_num + led_i][1] / 255 + ',' + leds[led_num + led_i][2] / 255 + '__';
        }
        //deb = deb + "\n";
    }
    //console.log("power:" + power + ' ' + deb);
    return Math.round(power*10)/10;
}
function scale_power(scale) {
    let led_num;
    for (let step_i = 0; step_i < NUM_STEPS; step_i++) {
        led_num = step_i * POINTS_PER_STEP;
        for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            leds[led_num + led_i][0] = leds[led_num + led_i][0] * scale;
            leds[led_num + led_i][1] = leds[led_num + led_i][1] * scale;
            leds[led_num + led_i][2] = leds[led_num + led_i][2] * scale;
        }
    }
}

function animate_loop() {
    max_animation_frame = 0;


    /*if (animation_mode == 5) {
        if (check_frames(1,1,4)) {
            draw_step2(0, Red, Blue);
            draw_step2(1, Blue, Green);
            draw_step2(2, Green, Red);

            draw_step3(4, Red, Green, Blue);
            draw_step3(5, Green, Blue, Red);
            draw_step3(6, Blue, Red, Green);

            draw_step2(8, Red, Blue);
            draw_step2(9, Blue, Green);
            draw_step2(10, Green, Red);

            draw_step3(11, Red, Green, Blue);
            draw_step3(12, Green, Blue, Red);
            draw_step3(13, Blue, Red, Green);

            draw_step(15, [ Red, Green, Blue, Red, Green, Blue, Red, Green, Blue]);

            //show_debug(key_frames + ': ' + key_frame + ' >>> ' + progress*MAX_ILLUM + ' M:' + max_animation_frame);
        }
        if (check_frames(4,300,4)) {
            shift_step(8, 1);
            shift_step(9, 1);
            shift_step(10, 1);

            shift_step(11, -1);
            shift_step(12, -1);
            shift_step(13, -1);
            //show_debug(key_frames + ': ' + key_frame + ' >>> ' + progress*MAX_ILLUM + ' M:' + max_animation_frame);
        }
    }*/

    if (animation_mode === 1) {
        //slow gradient wave
        let waves_count = 4;
        let frames_per_wave = 128;
        let wave_color1 = main_color1;
        let wave_color2 = main_color2;

        for (let wave_i=0; wave_i<waves_count; wave_i++) {
            if (check_frames( wave_i*frames_per_wave + 1, wave_i*frames_per_wave + 32, 4)) {
                fill_step(first_step, CRGB(progress * wave_color1[0], progress * wave_color1[1], progress * wave_color1[2]));
            }
            if (check_frames(wave_i*frames_per_wave + 32, wave_i*frames_per_wave + 64, 4)) {
                fill_step(first_step, CRGB(degress * wave_color1[0] + progress * wave_color2[0],
                    degress * wave_color1[1] + progress * wave_color2[1],
                    degress * wave_color1[2] + progress * wave_color2[2]));
            }
            if (check_frames(wave_i*frames_per_wave + 64, wave_i*frames_per_wave + 96, 4)) {
                fill_step(first_step, CRGB(degress * wave_color2[0], degress * wave_color2[1], degress * wave_color2[2]));
            }
        }
        if (check_frames(4, (waves_count-1)*frames_per_wave + 128 + 16, 4)) {
            move_all();
        }
    }

    if (animation_mode === 2) {
        //slow ball
        let wave_color1 = main_color1;
        let seconds = 10;
        let rolls_per_sec = 2; // 1,2,4
        let max_balls = 10;
        let back_color = calc_back_color();
        //console.log('frames_for_1step',rolls_per_sec, FRAME_MS, frames_for_1step);

        if (animation_frame === 1) {
            for (let i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        //1st ball
        for (let ball_i=0; ball_i<max_balls; ball_i++) {
            if (check_frames(ball_i * FPS/rolls_per_sec*2 +1, ball_i * FPS/rolls_per_sec*2 +FPS/rolls_per_sec/2 +1,4)) {
                draw_step3(first_step, back_color,
                    CRGB(wave_color1[0]/4 + progress*(wave_color1[0]-wave_color1[0]/4),
                        wave_color1[1]/4 + progress*(wave_color1[1]-wave_color1[1]/4),
                        wave_color1[2]/4 + progress*(wave_color1[2]-wave_color1[2]/4)),
                    back_color);
            }
            if (check_frames(ball_i * FPS/rolls_per_sec*2 +FPS/rolls_per_sec/2 +1, ball_i * FPS/rolls_per_sec*2 +FPS/rolls_per_sec +1,4)) {
                draw_step3(first_step, back_color,
                    CRGB(wave_color1[0]/4 + degress*(wave_color1[0]-wave_color1[0]/4),
                        wave_color1[1]/4 + degress*(wave_color1[1]-wave_color1[1]/4),
                        wave_color1[2]/4 + degress*(wave_color1[2]-wave_color1[2]/4)),
                    back_color);
                //show_debug(' >>> ' + ' M:' + max_animation_frame + '  ' + animation_frame);
            }
        }

        if (check_frames(2,(seconds+1) * FPS,4)) {
            show_debug(' >>> ' + ' M:' + max_animation_frame + '  ' + animation_frame + ' s:' + Math.round(animation_frame/FPS*10)/10);
            move_all();
        }
    }

    if (animation_mode === 3) {
        //single color wave
        let wave_color1 = main_color1;
        let back_color = Black; //calc_back_color();
        let speedon = 16;
        let speedoff = 32;

        if (animation_frame == 1) {
            for (let i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        if (direction === UP){
            for (let i = 0; i < NUM_STEPS; i++) {
                if (check_frames(i*speedon,i*speedon+32,4)) {
                    draw_step3(i,
                        back_color,
                        CRGB(progress*wave_color1[0],progress*wave_color1[1],progress*wave_color1[2]),
                        back_color);
                }

                if (check_frames(i*speedon+32,i*speedon+64,4)) {
                    draw_step3(i,
                        CRGB(progress*wave_color1[0],progress*wave_color1[1],progress*wave_color1[2]),
                        wave_color1,
                        CRGB(progress*wave_color1[0], progress*wave_color1[1], progress*wave_color1[2]));
                }
                if (i>0){
                    if (check_frames(i*speedoff+64,i*speedoff+96,4)) {
                        //console.log('degress ' + degress + ' ' + progress + ' ' + key_frame + '/' + key_frames + '_' + animation_frame + '   ' + (i*speedoff+64) +'...'+ (i*speedoff+96));
                        //draw_step3(i-1, Red, CRGB(degress*MAX_ILLUM, 0, 0), Red);
                        draw_step3(i-1,
                            wave_color1,
                            CRGB(degress*wave_color1[0],degress*wave_color1[1],degress*wave_color1[2]),
                            wave_color1);
                    }

                    if (check_frames(i*speedoff+96,i*speedoff+128,4)) {
                        draw_step3(i-1,
                            CRGB(degress*wave_color1[0], degress*wave_color1[1], degress*wave_color1[2]),
                            back_color,
                            CRGB(degress*wave_color1[0], degress*wave_color1[1], degress*wave_color1[2]));
                    }
                }

            }

        } else {
            for (let i = NUM_STEPS-1; i >= 0; i--) {
                if (check_frames((NUM_STEPS-i-1)*speedon,(NUM_STEPS-i-1)*speedon+32,4)) {
                    draw_step3(i,
                        back_color,
                        CRGB(progress*wave_color1[0],progress*wave_color1[1],progress*wave_color1[2]),
                        back_color);
                }

                if (check_frames((NUM_STEPS-i-1)*speedon+32,(NUM_STEPS-i-1)*speedon+64,4)) {
                    draw_step3(i,
                        CRGB(progress*wave_color1[0],progress*wave_color1[1],progress*wave_color1[2]),
                        wave_color1,
                        CRGB(progress*wave_color1[0],progress*wave_color1[1],progress*wave_color1[2]));
                }
                if (i<NUM_STEPS){
                    if (check_frames((NUM_STEPS-i-1)*speedoff+64,(NUM_STEPS-i-1)*speedoff+96,4)) {
                        draw_step3(i+1,
                            wave_color1,
                            CRGB(degress*wave_color1[0], degress*wave_color1[1], degress*wave_color1[2]),
                            wave_color1);
                    }

                    if (check_frames((NUM_STEPS-i-1)*speedoff+96,(NUM_STEPS-i-1)*speedoff+128,4)) {
                        draw_step3(i+1,
                            CRGB(degress*wave_color1[0], degress*wave_color1[1], degress*wave_color1[2]),
                            back_color,
                            CRGB(degress*wave_color1[0], degress*wave_color1[1], degress*wave_color1[2]));
                    }
                }

            }
        }
    }

    if (animation_mode === 4) {
        //fast waves
        let wave_color1 = main_color1;
        let seconds = 6;
        let rolls_per_sec = 2; // 1,2,4
        let back_color = calc_back_color();
        let frames_for_1step = FPS / rolls_per_sec / NUM_STEPS;

        if (animation_frame == 1) {
            for (let i = 0; i < NUM_STEPS; i++) {
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
            fill_step(step_on_prev1, CRGB(wave_color1[0]/1.5, wave_color1[1]/1.5, wave_color1[2]/1.5));
            fill_step(step_on, wave_color1);
        }
        if (check_frames(1,seconds*FPS,5)) {
            //do nothing, just set max_animation_frame = times*FPS
        }

    }

    if (animation_mode === 5) {
        //worms
        let seconds = 5;
        max_animation_frame = seconds*FPS;
        let frames_per_step = 3;
        let worm_len = 3;
        let worm_max_age = 6 + worm_len-1;
        let worm_count = 2;
        let frames_per_spawn = worm_max_age/2 *frames_per_step;
        let worm_color = main_color1;
        let back_color = calc_back_color();

        if (animation_frame === 1) {
            worms = [];
            for (let i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        // spawn
        if (animation_frame % frames_per_spawn === 1) {
            for (let i = 0; i < worm_count; i++) {
                let point;
                let step;
                let loop_counter = 0; let d = 0;
                while (loop_counter <= 50) {
                    point = random(0, POINTS_PER_STEP - 1);
                    if (direction === UP) {
                        step = random(0, NUM_STEPS - 1 - worm_max_age / 2);
                    } else {
                        step = random( worm_max_age / 2, NUM_STEPS - 1);
                    }
                    let d_min = 99;
                    for (let worm_i = 0; worm_i < worms.length; worm_i++) {
                        let age_i = (animation_frame - worms[worm_i].animation_frame) / frames_per_step;
                        if (direction === UP) {
                            d = calc_distance(step, point, worms[worm_i].step + age_i, worms[worm_i].point);
                        } else {
                            d = calc_distance(step, point, worms[worm_i].step - age_i, worms[worm_i].point);
                        }
                        if (d < d_min) {
                            d_min = d;
                        }
                    }
                    if (d_min >= 2 || worms.length === 0) {
                        break;
                    }
                    loop_counter++;
                }
                //console.log('loop_counter', worms.length, loop_counter);
                worms.push({step: step, point: point, animation_frame: animation_frame});
            }
        }

        if (animation_frame % frames_per_step === 1) {
            // die
            //for (let worm_i = 0; worm_i < worms.length; worm_i++) {
            for (let worm_i = worms.length-1; worm_i >= 0; worm_i--) {
                let age_i = (animation_frame - worms[worm_i].animation_frame) / frames_per_step;
                if (age_i >= worm_max_age) {
                    for (let a = 0; a < worm_len; a++) {
                        if (direction === UP) {
                            fill_point(worms[worm_i].step + age_i - a - 1, worms[worm_i].point, back_color);
                        } else {
                            fill_point(worms[worm_i].step - age_i + a + 1, worms[worm_i].point, back_color);
                        }
                    }
                    worms.splice(worm_i, 1);
                }
            }
            // draw
            for (let worm_i = 0; worm_i < worms.length; worm_i++) {
                let age_i = (animation_frame - worms[worm_i].animation_frame) / frames_per_step;
                if (age_i <= worm_max_age - worm_len) {
                    if (direction === UP) {
                        fill_point(worms[worm_i].step + age_i, worms[worm_i].point, worm_color);
                    } else {
                        fill_point(worms[worm_i].step - age_i, worms[worm_i].point, worm_color);
                    }
                }
                if (direction === UP) {
                    fill_point(worms[worm_i].step + age_i - worm_len, worms[worm_i].point, back_color);
                } else {
                    fill_point(worms[worm_i].step - age_i + worm_len, worms[worm_i].point, back_color);
                }
            }
        }

    }

    if (animation_mode === 6) {
        //music vertical single color
        let seconds = 3600;
        //max_animation_frame = seconds*FPS;
        let frames_per_pick_down = 8;
        let main_color = main_color1;
        main_color = CRGB(main_color[0]/1.5, main_color[1]/1.5, main_color[2]/1.5);
        let back_color = Black; //calc_back_color();
        let pic_color = main_color2;
        let use_pick = true;

        // Преобразуем значение от 0-100 в диапазон от 0-16
        let fill_steps = Math.round((globalMicValue / 100) * NUM_STEPS) - 1;
        //console.log(fill_steps);

        if (fill_steps >= step_on_prev1) {
            step_on_prev1 = fill_steps;
        } else {
            if (animation_frame % frames_per_pick_down === 1 && step_on_prev1 >= 0) {
                step_on_prev1--;
            }
        }
        //console.log(fill_steps, step_on_prev1);

        for (let i = 0; i < NUM_STEPS; i++) {
            if (i <= fill_steps) {
                fill_step(i, main_color);
            } else {
                fill_step(i, back_color);
            }
            if (use_pick && i === step_on_prev1) {
                fill_step(i, pic_color);
            }
        }
    }

    if (animation_mode === 7) {
        //music vertical gradient
        let seconds = 3600;
        //max_animation_frame = seconds*FPS;

        if (animation_frame % 2 === 0) {
            let back_color = Black;
            let hue1 = animation_frame % 360; // цикл по кругу оттенков 0–359
            let hue2 = (animation_frame + 270) % 360;
            pal_color1 = hslToCRGB(hue1 / 360, 1, 0.5); // s=1, l=0.5 — яркие чистые цвета
            pal_color2 = hslToCRGB(hue2 / 360, 1, 0.5);

            // Преобразуем значение от 0-100 в диапазон от 0-16
            let fill_steps = Math.round((globalMicValue / 100) * NUM_STEPS) - 1;
            //console.log(hue1, hue2, pal_color1, pal_color2);

            for (let step_i = 0; step_i < NUM_STEPS; step_i++) {
                if (step_i <= fill_steps) {
                    let pal_color_i = CRGB(
                        Math.round(pal_color1[0] * step_i / fill_steps + pal_color2[0] * (fill_steps - step_i) / fill_steps),
                        Math.round(pal_color1[1] * step_i / fill_steps + pal_color2[1] * (fill_steps - step_i) / fill_steps),
                        Math.round(pal_color1[2] * step_i / fill_steps + pal_color2[2] * (fill_steps - step_i) / fill_steps)
                    );

                    fill_step(step_i, pal_color_i);
                } else {
                    fill_step(step_i, back_color);
                }
            }
        }
    }

    if (animation_mode === 8) {
        //music horizontal single color
        let seconds = 3600;
        //max_animation_frame = seconds*FPS;

        let frames_per_pick_down = 8;
        let main_color = main_color1;
        main_color = CRGB(main_color[0]/1.5, main_color[1]/1.5, main_color[2]/1.5);
        let back_color = Black; //calc_back_color();
        let pic_color = main_color2;
        let use_pick = false;

        // Преобразуем значение от 0-100 в диапазон от 0-9
        let fill_steps = Math.round((globalMicValue / 100) * POINTS_PER_STEP) - 1;
        //console.log(fill_steps);

        if (fill_steps >= step_on_prev1) {
            step_on_prev1 = fill_steps;
        } else {
            if (animation_frame % frames_per_pick_down === 1 && step_on_prev1 >= 0) {
                step_on_prev1--;
            }
        }
        //console.log(fill_steps, step_on_prev1);

        for (let step_i = 0; step_i < NUM_STEPS; step_i++) {
            for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
                if (led_i <= fill_steps) {
                    leds[step_i * POINTS_PER_STEP + led_i] = main_color;
                } else {
                    leds[step_i * POINTS_PER_STEP + led_i] = back_color;
                }
                if (use_pick && led_i === step_on_prev1) {
                    leds[step_i * POINTS_PER_STEP + led_i] = pic_color;
                }
            }
        }
    }

    if (animation_mode === 9) {
        //music horizontal gradient
        let seconds = 3600;
        //max_animation_frame = seconds*FPS;

        if (animation_frame % 2 === 0) {
            let back_color = Black;
            let hue1 = animation_frame % 360; // цикл по кругу оттенков 0–359
            let hue2 = (animation_frame + 270) % 360;
            pal_color1 = hslToCRGB(hue1 / 360, 1, 0.5); // s=1, l=0.5 — яркие чистые цвета
            pal_color2 = hslToCRGB(hue2 / 360, 1, 0.5);

            // Преобразуем значение от 0-100 в диапазон от 0-16
            let fill_steps = Math.round((globalMicValue / 100) * POINTS_PER_STEP) - 1;
            //console.log(hue1, hue2, pal_color1, pal_color2);

            for (let step_i = 0; step_i < NUM_STEPS; step_i++) {
                for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
                    if (led_i <= fill_steps) {
                        let pal_color_i = CRGB(
                            Math.round(pal_color1[0] * led_i / fill_steps + pal_color2[0] * (fill_steps - led_i) / fill_steps),
                            Math.round(pal_color1[1] * led_i / fill_steps + pal_color2[1] * (fill_steps - led_i) / fill_steps),
                            Math.round(pal_color1[2] * led_i / fill_steps + pal_color2[2] * (fill_steps - led_i) / fill_steps)
                        );

                        leds[step_i * POINTS_PER_STEP + led_i] = pal_color_i;
                    } else {
                        leds[step_i * POINTS_PER_STEP + led_i] = back_color;
                    }
                }
            }
        }
    }

    if (animation_mode === 10) {
        //snakes
        let frames_per_step = 2;
        let worm_len = POINTS_PER_STEP*5;
        let frames_per_spawn = worm_len * frames_per_step;
        let worm_color = main_color1;
        let back_color = calc_back_color();
        let max_snakes = 8;
        max_animation_frame = (max_snakes+1) * frames_per_spawn;

        if (animation_frame === 1) {
            for (let i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        if (animation_frame % frames_per_step === 1) {
            //spawn
            if (animation_frame <= max_animation_frame - 3*frames_per_spawn) {
                let point = 0;
                let step;
                if (direction === UP) {
                    step = 0;
                } else {
                    step = NUM_STEPS - 1;
                }
                let frames_len = frames_per_spawn - 1 * frames_per_step * POINTS_PER_STEP;
                let cur_len = animation_frame % frames_per_spawn;
                let pal_color_i = CRGB(
                    Math.max(Math.round(back_color[0] * cur_len / frames_len + worm_color[0] * (frames_len - cur_len) / frames_len), back_color[0]),
                    Math.max(Math.round(back_color[1] * cur_len / frames_len + worm_color[1] * (frames_len - cur_len) / frames_len), back_color[1]),
                    Math.max(Math.round(back_color[2] * cur_len / frames_len + worm_color[2] * (frames_len - cur_len) / frames_len), back_color[2])
                );
                fill_point(step, point, pal_color_i);
                //console.log('step', step, point);
            }

            // draw
            if (direction === UP) {
                for (let step_i = NUM_STEPS-1; step_i > 0; step_i -= 2) {
                    shift_step(step_i, -1);
                    copy_point(step_i, POINTS_PER_STEP - 1, step_i-1, POINTS_PER_STEP - 1);
                    shift_step(step_i-1, 1);
                    if (step_i > 1) {
                        copy_point(step_i-1, 0, step_i-2, 0);
                    }
                }
            } else { //DOWN
                for (let step_i = 0; step_i < NUM_STEPS - 1; step_i += 2) {
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

    if (animation_mode === 11) {
        //sine
        let wave_color1 = main_color1;
        let seconds = 6;
        let rolls_per_sec = 64; // 1,2,4
        let back_color = calc_back_color();

        if (animation_frame === 1) {
            sine_i = 0;
            for (let i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        if (check_frames(2,seconds*FPS,4)) {
            move_all();
        }
        if (check_frames(2,seconds*FPS,4)) {
            let points = structuredClone(sine_mask[sine_i]);

            for (let i=0; i<POINTS_PER_STEP; i++) {
                points[i] = CRGB(
                    Math.round((points[i][0] * wave_color1[0])/256),
                    Math.round((points[i][1] * wave_color1[1])/256),
                    Math.round((points[i][2] * wave_color1[2])/256));
            }

            if (direction === UP) {
                draw_step(first_step, points);
            } else {
                draw_step(NUM_STEPS-1, points);
            }
            sine_i++;
            if (sine_i===sine_mask.length) {
                sine_i = 0;
            }
        }
    }

    if (animation_mode === 12) {
        //arrow
        let arrow_color1 = main_color1;
        let seconds = 12;
        let rolls_per_sec = 64; // 1,2,4
        let back_color = calc_back_color();

        if (animation_frame === 1) {
            sine_i = 0;
            for (let i = 0; i < NUM_STEPS; i++) {
                fill_step(i, back_color);
            }
        }

        if (check_frames(2,seconds*FPS,4)) {
            move_all();
        }

        if (check_frames(1,seconds*FPS,4)) {
            // Создаем локальную копию массива цветов
            let points = structuredClone(arrow_mask[sine_i]);
            for (let i=0; i<POINTS_PER_STEP; i++) {
                points[i] = CRGB(
                    Math.round(points[i][0] * arrow_color1[0] /256),
                    Math.round(points[i][1] * arrow_color1[1] /256),
                    Math.round(points[i][2] * arrow_color1[2] /256));
            }

            if (direction === UP) {
                draw_step(first_step, points);
            } else {
                draw_step(NUM_STEPS-1, points);
            }
            sine_i++;
            if (sine_i===arrow_mask.length) { // Нужно знать длину массива sine_mask
                sine_i = 0;
            }
        }
    }

    show_debug('Max:' + max_animation_frame + '  f:' + animation_frame + '  Sec:' + Math.round(animation_frame/FPS*10)/10);


    //console.log('key_frames:', key_frames + ': ' + key_frame + ' >>> ' + progress*MAX_ILLUM + ' M:' + max_animation_frame);
    if (animation_frame % 8 === 1) {
        power = calc_power();
        power_scale = 1;
        //console.log("power:" + power);
        if (power > MAX_POWER_SUPPLY) {
            power_scale = MAX_POWER_SUPPLY / power;
            scale_power(power_scale);
            //console.log("power scaled from:" + power);
        }
        document.getElementById('power').innerText = 'power: ' + power.toString() + ' W scale:' + power_scale;
    }

    // check animation finish
    animation_frame++;
    if (animation_frame > max_animation_frame && !MODES_FOR_MUSIC.includes(animation_mode)) {
        finish_animation();
    }

}

function finish_animation() {
    animation_frame = 0;
    clear_all();
    if (work_mode === 2 || work_mode === 4) {
        //change next random animation_mode
        let next_animation_mode = MODES_FOR_RANDOM[Math.floor(Math.random() * MODES_FOR_RANDOM.length)];
        if (next_animation_mode === animation_mode) {
            next_animation_mode = MODES_FOR_RANDOM[Math.floor(Math.random() * MODES_FOR_RANDOM.length)];
            if (next_animation_mode === animation_mode) {
                next_animation_mode = MODES_FOR_RANDOM[Math.floor(Math.random() * MODES_FOR_RANDOM.length)];
            }
        }
        animation_mode = next_animation_mode;
        console.log('next animation_mode', animation_mode);
    }
    if (work_mode === 3 || work_mode === 4) {
        //change next random color
        let next_main_color1 = colors_list[Math.floor(Math.random() * colors_list.length)];
        if (next_main_color1 === main_color1) {
            next_main_color1 = colors_list[Math.floor(Math.random() * colors_list.length)];
            if (next_main_color1 === main_color1) {
                next_main_color1 = colors_list[Math.floor(Math.random() * colors_list.length)];
            }
        }
        main_color1 = next_main_color1;

        main_color2 = colors_list[Math.floor(Math.random() * colors_list.length)];
        if (main_color2 === main_color1) {
            main_color2 = colors_list[Math.floor(Math.random() * colors_list.length)];
            if (main_color2 === main_color1) {
                main_color2 = colors_list[Math.floor(Math.random() * colors_list.length)];
            }
        }
        console.log('next colors', main_color1, main_color2);
    }
}

function check_frames(frame_from, frame_to, key_frame_interval) {
    if (max_animation_frame < frame_to) {
        max_animation_frame = frame_to;
    }

    if (animation_frame >= frame_from && animation_frame <= frame_to) {
        if (animation_frame % key_frame_interval === 1 || frame_from === frame_to) {
            key_frames = intdiv(frame_to - frame_from - key_frame_interval, key_frame_interval);
            key_frame = intdiv(animation_frame - frame_from, key_frame_interval); // 0..key_frames-1
            if (key_frames < 1) {
                key_frames = 1;
            }
            progress = key_frame/key_frames; // 0..1
            //console.log('key_frames:', key_frames + ': ' + key_frame + ' >>> ' + progress*255 + ' ' + max_animation_frame);
            if (progress < 0.03) {progress = 0}
            if (progress > 1) {progress = 1}
            degress = 1 - progress;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}
function copy_step(step_from, step_to) {
    for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
        leds[step_to * POINTS_PER_STEP + led_i] = leds[step_from * POINTS_PER_STEP + led_i];
    }
}
function copy_point(step, point, from_step, from_point) {
    if (step >= 0 && step < NUM_STEPS && point >= 0 && point < POINTS_PER_STEP) {
        if (from_step >= 0 && from_step < NUM_STEPS && from_point >= 0 && from_point < POINTS_PER_STEP) {
            leds[step * POINTS_PER_STEP + point] = leds[from_step * POINTS_PER_STEP + from_point];
        }
    }
}
function shift_step(step, shift_points) {
    if (shift_points >= POINTS_PER_STEP) {
        shift_points = shift_points%POINTS_PER_STEP;
    }
    if (shift_points <= -POINTS_PER_STEP) {
        shift_points = shift_points%POINTS_PER_STEP;
    }

    if (shift_points > 0) {
        //shift from left to right (from 0 to POINTS_PER_STEP)
        for (let led_i = POINTS_PER_STEP -shift_points -1; led_i >= 0; led_i = led_i - shift_points) {
            copy_point(step, led_i + shift_points, step, led_i);
            //fill_point(step, led_i + 1, leds[step * POINTS_PER_STEP + led_i]);
        }
    }
    if (shift_points < 0) {
        //shift from right to left (from POINTS_PER_STEP to 0)
        for (let led_i = Math.abs(shift_points); led_i < POINTS_PER_STEP; led_i = led_i - shift_points) {
            copy_point(step, led_i + shift_points, step, led_i);
        }
    }
}
function draw_step(step, points) {
    if (step >= 0 && step < NUM_STEPS) {
        for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            if (points[led_i]) {
                leds[step * POINTS_PER_STEP + led_i] = points[led_i];
            } else {
                leds[step * POINTS_PER_STEP + led_i] = Black;
            }
        }
    }
}
function illum_step(step, illum) {
    let led_num;
    if (step >= 0 && step < NUM_STEPS) {
        for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            led_num = step * POINTS_PER_STEP + led_i;
            leds[led_num] = CRGB(leds[led_num][0] + illum, leds[led_num][1] + illum, leds[led_num][2] + illum);
        }
    }
}
function fill_step(step, color) {
    if (step >= 0 && step < NUM_STEPS) {
        for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            leds[step * POINTS_PER_STEP + led_i] = color;
        }
    }
}

function fill_point(step, point, color) {
    if (step >= 0 && step < NUM_STEPS) {
        if (point >= 0 && point < POINTS_PER_STEP) {
            leds[step * POINTS_PER_STEP + point] = CRGB(color[0],color[1],color[2]);
        }
    }
}
function draw_step2(step, color1, color2) {
    if (step >= 0 && step < NUM_STEPS) {
        let max_num = POINTS_PER_STEP - 1;
        for (let led_i = 0; led_i < POINTS_PER_STEP; led_i++) {
            leds[step * POINTS_PER_STEP + led_i] = CRGB(
                color1[0] * (max_num - led_i) / max_num + color2[0] * led_i / max_num,
                color1[1] * (max_num - led_i) / max_num + color2[1] * led_i / max_num,
                color1[2] * (max_num - led_i) / max_num + color2[2] * led_i / max_num
            );
        }
    }
}
function draw_step3(step, color1, color2, color3) {
    let max_num = center_num;
    if (step >= 0 && step < NUM_STEPS) {
        for (let led_i = 0; led_i < center_num; led_i++) {
            leds[step * POINTS_PER_STEP + led_i] = CRGB(
                color1[0] * (max_num - led_i) / max_num + color2[0] * led_i / max_num,
                color1[1] * (max_num - led_i) / max_num + color2[1] * led_i / max_num,
                color1[2] * (max_num - led_i) / max_num + color2[2] * led_i / max_num
            );
        }
        for (let led_i = center_num; led_i < POINTS_PER_STEP; led_i++) {
            leds[step * POINTS_PER_STEP + led_i] = CRGB(
                color2[0] * (max_num - led_i + center_num) / max_num + color3[0] * (led_i - center_num) / max_num,
                color2[1] * (max_num - led_i + center_num) / max_num + color3[1] * (led_i - center_num) / max_num,
                color2[2] * (max_num - led_i + center_num) / max_num + color3[2] * (led_i - center_num) / max_num
            );
        }
    }
}
function move_all() {
    if (direction == UP) {
        for (let step_i = NUM_STEPS-1; step_i > 0; step_i--) {
            copy_step(step_i - 1, step_i);
            //console.log('u',step_i - 1, step_i);
        }
    } else {
        for (let step_i = 0; step_i < NUM_STEPS-1; step_i++) {
            copy_step(step_i + 1, step_i);
            //console.log('d',step_i + 1, step_i);
        }
    }
}
function clear_all() {
    for (let step_i = 0; step_i < NUM_STEPS; step_i++) {
        fill_step(step_i, Black);
    }
}
function show_debug(debug) {
    document.getElementById('debug').innerText = debug;
}

function hslToCRGB(h, s, l) {
    let r, g, b;

    if (s === 0) {
        r = g = b = l; // achromatic
    } else {
        const hue2rgb = (p, q, t) => {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1 / 6) return p + (q - p) * 6 * t;
            if (t < 1 / 2) return q;
            if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
            return p;
        };

        const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        const p = 2 * l - q;
        r = hue2rgb(p, q, h + 1 / 3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1 / 3);
    }

    return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}

function calc_back_color() {
    let back_color = CRGB(main_color1[0]/4, main_color1[1]/4, main_color1[2]/4);
    return back_color;
}
