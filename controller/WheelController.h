
#ifndef WheelController_h
#define WheelController_h

namespace controller {

volatile float balance_output;
volatile float movement_output_l, movement_output_r;
volatile float wheel_output_l, wheel_output_r;

float theta_last;
float tau_last;
float custom_controller_gain = 32000;
bool use_custom_controller = true;
void updateBalance(float angular_position, float angular_velocity) {
    if (use_custom_controller) {
        // Own controller
        float theta = angular_position / 57.3f;
    
        float a0 = 2.075, a1 = -1.925, b0 = 0.6118125, b1 = -0.5881875;
        float tau = b0/a0 * theta + b1/a0 * theta_last - a1/a0 * tau_last;
    
        tau_last = tau;
        theta_last = theta;
    
        balance_output = tau / 2.0 * custom_controller_gain;
    } else {
        balance_output = 55 * angular_position + 0.75 * angular_velocity;
    }
}

unsigned int speed_control_period_count = 0;
double speed_filter, speed_filter_old, car_speed_integral, speed_control_output, rotation_control_output;
double wheel_speed_l_counter = 0;
double wheel_speed_r_counter = 0;
void updateMovement(float wheel_speed_l, float wheel_speed_r, float angular_vel_z) {
    wheel_speed_l_counter += wheel_speed_l;
    wheel_speed_r_counter += wheel_speed_r;
    speed_control_period_count++;
    if (speed_control_period_count >= 8) {
        speed_control_period_count = 0;
        double car_speed = (wheel_speed_l_counter + wheel_speed_r_counter) * 0.5;
        wheel_speed_l_counter = 0;
        wheel_speed_r_counter = 0;
        speed_filter = speed_filter_old * 0.7 + car_speed * 0.3;
        speed_filter_old = speed_filter;
        car_speed_integral += speed_filter;
        car_speed_integral = constrain(car_speed_integral, -3000, 3000);
        speed_control_output = -10 * speed_filter - 0.26*car_speed_integral;
        rotation_control_output = 0.5 * angular_vel_z;
    }
    movement_output_l = -speed_control_output - rotation_control_output;
    movement_output_r = -speed_control_output + rotation_control_output;
}
void updateController(float angular_position, float angular_velocity, float wheel_speed_l, float wheel_speed_r, float angular_vel_z) {
    updateBalance(angular_position, angular_velocity);
    updateMovement(wheel_speed_l, wheel_speed_r, angular_vel_z);
    wheel_output_l = balance_output + movement_output_l;
    wheel_output_r = balance_output + movement_output_r;
}

void setCustomController(float gain) {
    custom_controller_gain = gain;
    use_custom_controller = true;
}

void setDefaultController() {
    use_custom_controller = false;
}

void reset() {
    car_speed_integral = 0;
    speed_filter = 0;
    speed_filter_old = 0;
    speed_control_output = 0;
    rotation_control_output = 0;
    balance_output = 0;
}

}
#endif
