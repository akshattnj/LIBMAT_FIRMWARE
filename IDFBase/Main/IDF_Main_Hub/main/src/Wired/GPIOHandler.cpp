/**
 * General shape of the hub
 * 
 * |------------------------------------|
 * |                                    |
 * |    ------------    ------------    |
 * |    | ?? ?? ?? |    | ?? ?? ?? |    | Charger Sensor Lock GPIO
 * |    |     3    |    |     1    |    | Slot Number
 * |    |   21-27  |    |   7-13   |    | Neopixel Order
 * |    |   0x00E  |    |   0x00C  |    | Slot CAN ID
 * |    ------------    ------------    |
 * |                                    |
 * |    ------------    ------------    |
 * |    | ?? ?? ?? |    | ?? ?? ?? |    | Charger Sensor Lock GPIO
 * |    |     2    |    |     0    |    | Slot Number
 * |    |   14-20  |    |    0-6   |    | Neopixel Order
 * |    |   0x00D  |    |   0x00B  |    | Slot CAN ID
 * |    ------------    ------------    |
 * |                                    |
 * |------------------------------------|
 * 
 * Master CAN ID 0x00A
 * General response ping ID 0xFFF
 * 
 * TODO:
 * 1) Identify initial state
 *     a) General response ping
 *     b) Accept responses for 3 seconds
 *     c) Map battries to slots
 * 2) Initialise GPIO based on initial state
 * 3) Handle swap
 *     a) Identify slot with swappable battery --->>> Error Path - No swappable battery
 *     b) Assign CAN ID to incoming battery
 *     c) Scan for incoming battery
 *     d) Scan for incoming battery door closed
 *     e) Change charger states
 *     f) Scan for outgoing battery removal
 *     g) Scan for outgoing battery gate closed
*/

#include "GPIOHandler.h"

namespace GPIO
{

}