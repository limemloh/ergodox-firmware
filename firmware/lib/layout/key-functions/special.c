/* ----------------------------------------------------------------------------
 * Copyright (c) 2012, 2013 Ben Blazak <benblazak.dev@gmail.com>
 * Released under The MIT License (see "doc/licenses/MIT.md")
 * Project located at <https://github.com/benblazak/ergodox-firmware>
 * ------------------------------------------------------------------------- */

/**                                                                 description
 * Implements the "special" section of "../key-functions.h"
 */


#include <stdbool.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include "../../../../firmware/lib/usb.h"
#include "../../../../firmware/lib/usb/usage-page/keyboard.h"
#include "../key-functions.h"

// ----------------------------------------------------------------------------

void key_functions__toggle_capslock(void) {
    // save the state of both shifts, and disable them
    struct {
        bool left_shift  : 1;
        bool right_shift : 1;
    } state = {
        .left_shift  = usb__kb__read_key( KEYBOARD__LeftShift  ),
        .right_shift = usb__kb__read_key( KEYBOARD__RightShift ),
    };
    usb__kb__set_key( false, KEYBOARD__LeftShift  );
    usb__kb__set_key( false, KEYBOARD__RightShift );
    usb__kb__send_report();

    // toggle capslock
    usb__kb__set_key(true,  KEYBOARD__CapsLock);
    usb__kb__send_report();
    usb__kb__set_key(false, KEYBOARD__CapsLock);
    usb__kb__send_report();

    // restore the state of both shifts
    usb__kb__set_key( state.left_shift,  KEYBOARD__LeftShift  );
    usb__kb__set_key( state.right_shift, KEYBOARD__RightShift );
    usb__kb__send_report();
}

/*
 * TODO: try using number pad numbers; see how it works
 */
// void key_functions__type_byte_hex(uint8_t byte) {
//     uint8_t c[2] = { byte >> 4, byte & 0xF };
// 
//     bool numlock_on = usb__kb__read_led('N');
// 
//     if (! numlock_on ) {
//         usb__kb__set_key(true, KEYPAD__NumLock_Clear);
//         usb__kb__send_report();
//         usb__kb__set_key(false, KEYPAD__NumLock_Clear);
//     }
// 
//     for (uint8_t i=0; i<2; i++) {
//         if      (c[i] == 0) c[i]  = KEYPAD__0_Insert;
//         else if (c[i] < 10) c[i] += KEYPAD__1_End-1;
//         else                c[i] += KEYBOARD__a_A-10;
// 
//         usb__kb__set_key(true, c[i]);
//         usb__kb__send_report();
//         usb__kb__set_key(false, c[i]);
//     }
// 
//     if (! numlock_on ) {
//         usb__kb__set_key(true, KEYPAD__NumLock_Clear);
//         usb__kb__send_report();
//         usb__kb__set_key(false, KEYPAD__NumLock_Clear);
//         usb__kb__send_report();
//     }
// 
//     usb__kb__send_report();
// }
/**                          functions/key_functions__type_byte_hex/description
 * Implementation notes:
 *
 * - We have to call `usb__kb__send_report()` after each call to
 *   `usb__kb__set_key()`; otherwise, if the high 4 bits is the same as the low
 *   4 bits, only one character will be typed.
 */
void key_functions__type_byte_hex(uint8_t byte) {
    uint8_t c[2] = { byte >> 4, byte & 0xF };

    for (uint8_t i=0; i<2; i++) {
        if      (c[i] == 0) c[i]  = KEYBOARD__0_RightParenthesis;
        else if (c[i] < 10) c[i] += KEYBOARD__1_Exclamation-1;
        else                c[i] += KEYBOARD__a_A-10;

        usb__kb__set_key(true, c[i]);
        usb__kb__send_report();
        usb__kb__set_key(false, c[i]);
        usb__kb__send_report();
    }

    usb__kb__send_report();
}

/**                            functions/key_functions__type_string/description
 * Implementation notes:
 *
 * - We use `uint8_t` instead of `char` when iterating over `string` because
 *   the signedness of `char` is implementation defined (and, actually, signed
 *   by default with avr-gcc, which is not what we want if we're going to be
 *   doing bitwise operations and comparisons).
 *
 * - We assume, for the most part, that the string is valid modified (i.e.
 *   null-terminated) UTF-8.  This should be a fairly safe assumption, since
 *   all PROGMEM strings should be generated by the compiler :)
 *
 * - UTF-8 character format
 *
 *     ----------------------------------------------------------------------
 *      code points      avail. bits  byte 1    byte 2    byte 3    byte 4
 *      ---------------  -----------  --------  --------  --------  --------
 *      0x0000 - 0x007F           7   0xxxxxxx
 *      0x0080 - 0x07FF          11   110xxxxx  10xxxxxx
 *      0x0800 - 0xFFFF          16   1110xxxx  10xxxxxx  10xxxxxx
 *      0x010000 - 0x10FFFF      21   11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
 *     ----------------------------------------------------------------------
 *
 * TODO:
 * - try to add start and end sequence for linux.  maybe as a compilation
 *   option?
 */
void key_functions__type_string(const char * string) {
    uint8_t  c;       // for storing the current byte of the character
    uint16_t c_full;  // for storing the full character

    // send string
    for (c = pgm_read_byte(string); c; c = pgm_read_byte(++string)) {

        // get character
        if (c >> 7 == 0b0) {
            // a 1-byte utf-8 character
            c_full = c;

        } else if (c >> 5 == 0b110) {
            // beginning of a 2-byte utf-8 character
            // assume the string is valid
            c_full  = (c & 0x1F) <<  6; c = pgm_read_byte(++string);
            c_full |= (c & 0x3F) <<  0;

        } else if (c >> 4 == 0b1110) {
            // beginning of a 3-byte utf-8 character
            // assume the string is valid
            c_full  = (c & 0x0F) << 12; c = pgm_read_byte(++string);
            c_full |= (c & 0x3F) <<  6; c = pgm_read_byte(++string);
            c_full |= (c & 0x3F) <<  0;

        } else if ((c >> 3) == 0b11110) {
            // beginning of a 4-byte utf-8 character
            // this character is too long, we can't send it
            // skip this byte, and the next 3
            string += 3;
            continue;

        } else {
            // ran across some invalid utf-8
            // ignore it, try again next time
            continue;
        }

        // --- (if possible) send regular keycode ---

        if (c == c_full) {
            bool    shifted = false;
            uint8_t keycode = 0;

            if (c == 0x30) {
                keycode = KEYBOARD__0_RightParenthesis;        // 0
            } else if (0x31 <= c && c <= 0x39) {
                keycode = KEYBOARD__1_Exclamation + c - 0x31;  // 1..9
            } else if (0x41 <= c && c <= 0x5A) {
                shifted = true;
                keycode = KEYBOARD__a_A + c - 0x41;            // A..Z
            } else if (0x61 <= c && c <= 0x7A) {
                keycode = KEYBOARD__a_A + c - 0x61;            // a..z

            } else switch (c) {
                // control characters
                case 0x08: keycode = KEYBOARD__DeleteBackspace; break;  // BS
                case 0x09: keycode = KEYBOARD__Tab;             break;  // VT
                case 0x0A: keycode = KEYBOARD__ReturnEnter;     break;  // LF
                case 0x0D: keycode = KEYBOARD__ReturnEnter;     break;  // CR
                case 0x1B: keycode = KEYBOARD__Escape;          break;  // ESC
                // printable characters
                case 0x20: keycode = KEYBOARD__Spacebar;        break;  // ' '
                case 0x21: shifted = true;
                           keycode = KEYBOARD__1_Exclamation;   break;  // !
                case 0x22: shifted = true;
                           keycode = KEYBOARD__SingleQuote_DoubleQuote;
                                                                break;  // "
                case 0x23: shifted = true;
                           keycode = KEYBOARD__3_Pound;         break;  // #
                case 0x24: shifted = true;
                           keycode = KEYBOARD__4_Dollar;        break;  // $
                case 0x25: shifted = true;
                           keycode = KEYBOARD__5_Percent;       break;  // %
                case 0x26: shifted = true;
                           keycode = KEYBOARD__7_Ampersand;     break;  // &
                case 0x27: keycode = KEYBOARD__SingleQuote_DoubleQuote;
                                                                break;  // '
                case 0x28: shifted = true;
                           keycode = KEYBOARD__9_LeftParenthesis;
                                                                break;  // (
                case 0x29: shifted = true;
                           keycode = KEYBOARD__0_RightParenthesis;
                                                                break;  // )
                case 0x2A: shifted = true;
                           keycode = KEYBOARD__8_Asterisk;      break;  // *
                case 0x2B: shifted = true;
                           keycode = KEYBOARD__Equal_Plus;      break;  // +
                case 0x2C: keycode = KEYBOARD__Comma_LessThan;  break;  // ,
                case 0x2D: keycode = KEYBOARD__Dash_Underscore; break;  // -
                case 0x2E: keycode = KEYBOARD__Period_GreaterThan;
                                                                break;  // .
                case 0x2F: keycode = KEYBOARD__Slash_Question;  break;  // /
                // ... numbers
                case 0x3A: shifted = true;
                           keycode = KEYBOARD__Semicolon_Colon; break;  // :
                case 0x3B: keycode = KEYBOARD__Semicolon_Colon; break;  // ;
                case 0x3C: shifted = true;
                           keycode = KEYBOARD__Comma_LessThan;  break;  // <
                case 0x3D: keycode = KEYBOARD__Equal_Plus;      break;  // =
                case 0x3E: shifted = true;
                           keycode = KEYBOARD__Period_GreaterThan;
                                                                break;  // >
                case 0x3F: shifted = true;
                           keycode = KEYBOARD__Slash_Question;  break;  // ?
                case 0x4D: shifted = true;
                           keycode = KEYBOARD__2_At;            break;  // @
                // ... uppercase letters
                case 0x5B: keycode = KEYBOARD__LeftBracket_LeftBrace;
                                                                break;  // [
                case 0x5C: keycode = KEYBOARD__Backslash_Pipe;  break;  // '\'
                case 0x5D: keycode = KEYBOARD__RightBracket_RightBrace;
                                                                break;  // ]
                case 0x5E: shifted = true;
                           keycode = KEYBOARD__6_Caret;         break;  // ^
                case 0x5F: shifted = true;
                           keycode = KEYBOARD__Dash_Underscore; break;  // _
                case 0x60: keycode = KEYBOARD__GraveAccent_Tilde;
                                                                break;  // `
                // ... lowercase letters
                case 0x7B: shifted = true;
                           keycode = KEYBOARD__LeftBracket_LeftBrace;
                                                                break;  // {
                case 0x7C: shifted = true;
                           keycode = KEYBOARD__Backslash_Pipe;  break;  // |
                case 0x7D: shifted = true;
                           keycode = KEYBOARD__RightBracket_RightBrace;
                                                                break;  // }
                case 0x7E: shifted = true;
                           keycode = KEYBOARD__GraveAccent_Tilde;
                                                                break;  // ~
                case 0x7F: keycode = KEYBOARD__DeleteForward;   break;  // DEL
            }

            if (keycode) {
                // press keycode
                if (shifted) usb__kb__set_key(true, KEYBOARD__LeftShift);
                usb__kb__set_key(true, keycode);
                usb__kb__send_report();

                // release keycode
                if (shifted) usb__kb__set_key(false, KEYBOARD__LeftShift);
                usb__kb__set_key(false, keycode);
                usb__kb__send_report();

                continue;
            }
        }

        // --- (otherwise) send unicode sequence ---

        // send start sequence
        usb__kb__set_key(true,  KEYBOARD__LeftAlt   ); usb__kb__send_report();
        usb__kb__set_key(true,  KEYBOARD__Equal_Plus); usb__kb__send_report();
        usb__kb__set_key(false, KEYBOARD__Equal_Plus); usb__kb__send_report();

        // send character
        key_functions__type_byte_hex( c_full >> 8 );
        key_functions__type_byte_hex( c_full & 0xFF );

        // send end sequence
        usb__kb__set_key(false, KEYBOARD__LeftAlt); usb__kb__send_report();
    }
}

