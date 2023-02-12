
#include "controller-config.h"

#include "logging/logging.h"

#include "controller/controller.h"


//
// START OF STEPPER TIMER STUFFS
//

[[nodiscard]] bool stepper_timer_handler(struct repeating_timer *t);
volatile uint64_t stepper_frame_count = 0L;

/**
 * Simple array for setting the address lines of the stepper latches
 */
static bool stepperAddressMapping[MAX_NUMBER_OF_STEPPERS][STEPPER_MUX_BITS] = {

        {false,     false,      false},     // 0
        {false,     false,      true},      // 1
        {false,     true,       false},     // 2
        {false,     true,       true},      // 3
        {true,      false,      false},     // 4
        {true,      false,      true},      // 5
        {true,      true,       false},     // 6
        {true,      true,       true}       // 7
};

//
// END OF STEPPER TIMER STUFFS
//



/**
 *
 * Callback for the stepper timer
 *
 * REMEMBER THAT THIS RUNS EVERY FEW MICROSECONDS! :)
 *
 * @param t the repeating timer
 * @return true
 */
bool stepper_timer_handler(struct repeating_timer *t) {

    // Keep track of which frame we're in
    stepper_frame_count++;

    // Look at each stepper we have and adjust if needed
    for(int i = 0; i < Controller::getNumberOfSteppersInUse(); i++) {

        Stepper *s = Controller::getStepper(i);
        uint8_t slot = s->getSlot();


        /*
         * Load this stepper's state
         */
        StepperState* state = s->state;


        // If this stepper is high, there's nothing else to do. Set it to low.
        if(state->isHigh) {

            state->isHigh = false;
            goto transmit;
        }

        // If we're at the position where we need to be, stop
        if(state->currentStep == state->desiredSteps) {
            goto end;
        }


        /*
         * So now we know we need to move. Let's figure out which direction.
         */

        if( state->currentStep < state->desiredSteps ) {

            state->currentDirection = false;
            state->currentStep++;
            state->isHigh = true;

            goto transmit;

        }

        // The only thing left is to move the other way
        state->currentDirection = true;
        state->currentStep--;
        state->isHigh = true;

        goto transmit;




        /*
         * Get the state of the latch for this stepper to match what we think it is
         */


        transmit:

        // Configure the address lines
        gpio_put(STEPPER_A0_PIN, stepperAddressMapping[slot][2]);
        gpio_put(STEPPER_A1_PIN, stepperAddressMapping[slot][1]);
        gpio_put(STEPPER_A2_PIN, stepperAddressMapping[slot][0]);


        gpio_put(STEPPER_DIR_PIN, state->currentDirection);
        gpio_put(STEPPER_STEP_PIN, state->isHigh);
        gpio_put(STEPPER_MS1_PIN, state->ms1State);
        gpio_put(STEPPER_MS2_PIN, state->ms2State);
        gpio_put(STEPPER_SLEEP_PIN, state->isAwake);        // Sleep is active low

        // Enable the latch
        gpio_put(STEPPER_LATCH_PIN, false);     // It's active low

        // Stall long enough to let the latch go! This about 380ns. The datasheet says it
        // needs 220ns to latch at 2v. (We run at 3.3v) The uint32_t executes faster than an
        // uint8_t! It surprised me to figure this out. :)
        volatile uint32_t j;
        for(j = 0; j < 3; j++) {}

        // Now that we've toggled everything, turn the latch back off
        gpio_put(STEPPER_LATCH_PIN, true);     // It's active low

        state->updatedFrame = stepper_frame_count;

        end:
        (void*)nullptr;

    }

    return true;
}