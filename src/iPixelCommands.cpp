#include "iPixelCommands.h"

namespace iPixelCommands {

    int checkRange(const char* name, int value, int minVal, int maxVal) {
        // If value is -1, use the minimum value as default
        if (value == -1) {
            Serial.printf("[iPixelCommands] %s not provided, using default: %d\n", name, minVal);
            return minVal;
        }

        if (value < minVal || value > maxVal) {
            Serial.print("EXCEPTION: ");
            Serial.print(name);
            Serial.print(" out of range (");
            Serial.print(minVal);
            Serial.print("-");
            Serial.print(maxVal);
            Serial.print("), got ");
            Serial.println(value);
            throw std::invalid_argument(std::string(name) + " out of range (" + std::to_string(minVal) + "-" + std::to_string(maxVal) + ") got " + std::to_string(value));
        }
        return value;
    }

    std::vector<uint8_t> setTime(int hour, int minute, int second) {
        hour = checkRange("Hour", hour, 0, 23);
        minute = checkRange("Minute", minute, 0, 59);
        second = checkRange("Second", second, 0, 59);

        std::vector<uint8_t> frame(8);
        frame[0] = 0x08;
        frame[1] = 0x00;
        frame[2] = 0x01;
        frame[3] = 0x80;
        frame[4] = (uint8_t)hour;
        frame[5] = (uint8_t)minute;
        frame[6] = (uint8_t)second;
        frame[7] = 0x00;

        return frame;
    }

    std::vector<uint8_t> setFunMode(bool value) {
        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x04;
        frame[3] = 0x01;
        frame[4] = value ? 0x01 : 0x00;

        return frame;
    }
        
    std::vector<uint8_t> setOrientation(int orientation) {
        orientation = checkRange("Orientation", orientation, 0, 2);

        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x06;
        frame[3] = 0x80;
        frame[4] = (uint8_t)orientation;

        return frame;
    }

    std::vector<uint8_t> clear() {
        std::vector<uint8_t> frame(4);
        frame[0] = 0x04;
        frame[1] = 0x00;
        frame[2] = 0x03;
        frame[3] = 0x80;

        return frame;
    }

    std::vector<uint8_t> setBrightness(int brightness) {
        brightness = checkRange("Brightness", brightness, 0, 100);

        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x04;
        frame[3] = 0x80;
        frame[4] = (uint8_t)brightness;

        return frame;
    }

    std::vector<uint8_t> setSpeed(int speed) {
        speed = checkRange("Speed", speed, 0, 100);

        std::vector<uint8_t> frame(4);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x03;
        frame[3] = (uint8_t)speed;

        return frame;
    }

    std::vector<uint8_t> ledOff() {
        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x07;
        frame[3] = 0x01;
        frame[4] = 0x00;

        return frame;
    }

    std::vector<uint8_t> ledOn() {
        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x07;
        frame[3] = 0x01;
        frame[4] = 0x01;

        return frame;
    }

    std::vector<uint8_t> deleteScreen(int screen) {
        screen = checkRange("Screen", screen, 0, 10);

        std::vector<uint8_t> frame(6);
        frame[0] = 0x07;
        frame[1] = 0x00;
        frame[2] = 0x02;
        frame[3] = 0x01;
        frame[4] = 0x00;
        frame[5] = (uint8_t)screen;

        return frame;
    }

    std::vector<uint8_t> setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        x = checkRange("X", x, 0, 255);
        y = checkRange("Y", y, 0, 255);
        r = checkRange("R", r, 0, 255);
        g = checkRange("G", g, 0, 255);
        b = checkRange("B", b, 0, 255);

        std::vector<uint8_t> frame(10);
        frame[0] = 0x0A;
        frame[1] = 0x00;
        frame[2] = 0x05;
        frame[3] = 0x01;
        frame[4] = 0x00;
        frame[5] = r;
        frame[6] = g;
        frame[7] = b;
        frame[8] = (uint8_t)x;
        frame[9] = (uint8_t)y;

        return frame;
    }

    std::vector<uint8_t> setClockMode(int style, int dayOfWeek, int year, int month, int day, bool showDate, bool format24) {
        style = checkRange("Style", style, 1, 8);
        dayOfWeek = checkRange("Day of Week", dayOfWeek, 1, 7);
        year = checkRange("Year", year, 0, 99);
        month = checkRange("Month", month, 1, 12);
        day = checkRange("Day", day, 1, 31);

        std::vector<uint8_t> frame(11);
        frame[0] = 0x0B;
        frame[1] = 0x00;
        frame[2] = 0x06;
        frame[3] = 0x01;
        frame[4] = (uint8_t)style;
        frame[5] = format24 ? 0x01 : 0x00;
        frame[6] = showDate ? 0x01 : 0x00;
        frame[7] = (uint8_t)year;
        frame[8] = (uint8_t)month;
        frame[9] = (uint8_t)day;
        frame[10] = (uint8_t)dayOfWeek;

        return frame;
    }

    std::vector<uint8_t> setRhythmLevelMode(int style, const int levels[11]) {
        style = checkRange("Style", style, 0, 4);
        for (int i = 0; i < 11; i++) {
            int level = checkRange("Level", levels[i], 0, 15);
            // Note: We can't modify the const array, so we just validate
        }

        std::vector<uint8_t> frame(5 + 11);
        frame[0] = 0x10;
        frame[1] = 0x00;
        frame[2] = 0x01;
        frame[3] = 0x02;
        frame[4] = (uint8_t)style;
        for (int i = 0; i < 11; i++)
            frame[5 + i] = (uint8_t)levels[i];

        return frame;
    }

    std::vector<uint8_t> setRhythmAnimationMode(int style, int frameNumber) {
        style = checkRange("Style", style, 0, 1);
        frameNumber = checkRange("Frame", frameNumber, 0, 7);

        std::vector<uint8_t> frame(6);
        frame[0] = 0x06;
        frame[1] = 0x00;
        frame[2] = 0x00;
        frame[3] = 0x02;
        frame[4] = (uint8_t)frameNumber;
        frame[5] = (uint8_t)style;

        return frame;
    }

    std::vector<uint8_t> setRhythmAnimationMode2(int style, int animationTime) {
        style = checkRange("Style", style, 0, 1);
        animationTime = checkRange("Animation Time", animationTime, 0, 7);

        std::vector<uint8_t> frame(6);
        frame[0] = 0x06;
        frame[1] = 0x00;
        frame[2] = 0x00;
        frame[3] = 0x02;
        frame[4] = (uint8_t)animationTime;
        frame[5] = (uint8_t)style;

        return frame;
    }

    std::vector<uint8_t> encodeText(const String& text, int matrix_height, uint8_t r, uint8_t g, uint8_t b) {
        std::vector<uint8_t> frame;
        uint8_t matrix_height_byte = (uint8_t)matrix_height;

        for (char character : text) {
            auto it = FONT_VCR_OSD_MONO_16PX.find(character);
            if (it == FONT_VCR_OSD_MONO_16PX.end()) continue;

            const FontChar& fontChar = it->second;
            std::vector<uint8_t> char_bytes;

            // Convert each 16-bit line to bytes
            for (uint16_t line_data : fontChar.data) {
                char_bytes.push_back((uint8_t)((line_data >> 8) & 0xFF));
                char_bytes.push_back((uint8_t)(line_data & 0xFF));
            }

            // Apply original transformations
            char_bytes = Helpers::invertFrames(char_bytes);
            char_bytes = Helpers::switchEndian(char_bytes);
            char_bytes = Helpers::logicReverseBitsOrder(char_bytes);

            uint8_t char_width_byte = (uint8_t)fontChar.width;

            if (!char_bytes.empty()) {
                // "80" = use per-character color
                frame.push_back(0x80);
                frame.push_back(r);
                frame.push_back(g);
                frame.push_back(b);
                frame.push_back(char_width_byte);
                frame.push_back(matrix_height_byte);
                frame.insert(frame.end(), char_bytes.begin(), char_bytes.end());
            }
        }

        return frame;
    }

    std::vector<uint8_t> sendText(const String& text, int animation, int save_slot, int speed, uint8_t r, uint8_t g, uint8_t b, int rainbow_mode, int matrix_height) {
        // --- Validation ---
        if (text.length() == 0 || text.length() > 100) return {};
        if (animation == 3 || animation == 4) return {};

        // --- Header calculation ---
        const uint16_t HEADER_1_MG = 0x1D;
        const uint16_t HEADER_3_MG = 0x0E;
        uint16_t header_gap = 0x06 + matrix_height * 0x02;

        uint16_t header_1_val = HEADER_1_MG + text.length() * header_gap;
        uint16_t header_3_val = HEADER_3_MG + text.length() * header_gap;

        std::vector<uint8_t> header;

        auto appendHeader16 = [&header](uint16_t val) {
            std::vector<uint8_t> temp = { 
                (uint8_t)((val >> 8) & 0xFF),
                (uint8_t)(val & 0xFF)
            };
            auto switched = Helpers::switchEndian(temp);
            header.insert(header.end(), switched.begin(), switched.end());
        };

        appendHeader16(header_1_val); //Byte 1-2
        header.push_back(0x00); //Byte 3
        header.push_back(0x01); //Byte 4
        header.push_back(0x00); //Byte 5
        appendHeader16(header_3_val); //Byte 6-7
        header.push_back(0x00); //Byte 8
        header.push_back(0x00); //Byte 9

        // --- Save slot ---
        uint16_t save_slot_val = (uint16_t)(save_slot);
        std::vector<uint8_t> save_slot_bytes = {
            (uint8_t)(save_slot_val & 0xFF),
            (uint8_t)((save_slot_val >> 8) & 0xFF)
        };
        save_slot_bytes = save_slot_bytes; //Byte 14-15

        // --- Payload ---
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)(text.length()));          // number of characters
        payload.push_back(0x00); payload.push_back(0x01); payload.push_back(0x01); // fixed prefix

        payload.push_back((uint8_t)(animation));
        payload.push_back((uint8_t)(speed));
        payload.push_back((uint8_t)(rainbow_mode));

        // Append "ffffff00000000" as bytes
        payload.push_back(0xFF); payload.push_back(0xFF); payload.push_back(0xFF);
        payload.push_back(0x00); payload.push_back(0x00); payload.push_back(0x00);
        payload.push_back(0x00);

        // Append encoded characters
        std::vector<uint8_t> chars_bytes = encodeText(text, matrix_height, r, g, b);
        payload.insert(payload.end(), chars_bytes.begin(), chars_bytes.end());

        // --- CRC ---
        std::vector<uint8_t> crc_bytes = Helpers::calculateCRC32Bytes(payload); //Byte 10-13

        // --- Assemble final message ---
        std::vector<uint8_t> result;
        result.insert(result.end(), header.begin(), header.end());
        result.insert(result.end(), crc_bytes.begin(), crc_bytes.end());
        result.insert(result.end(), save_slot_bytes.begin(), save_slot_bytes.end());
        result.insert(result.end(), payload.begin(), payload.end());

        return result;
    }

    std::vector<uint8_t> sendPNG(const std::vector<uint8_t>& pngData) {
        if (pngData.empty()) {
            throw std::invalid_argument("PNG data cannot be empty");
        }

        // Python implementation:
        // checksum = CRC32_checksum(png_hex)
        // size = get_frame_size(png_hex, 8)
        // return bytes.fromhex(f"{get_frame_size('FFFF020000' + size + checksum + '0065' + png_hex, 4)}020000{size}{checksum}0065{png_hex}")

        // This means:
        // 1. Calculate size of PNG data (as 8-char hex string, then endian-switched)
        // 2. Calculate CRC32 of PNG data
        // 3. Build inner: 'FFFF020000' + size + checksum + '0065' + png_hex
        // 4. Calculate frame_size of inner (as 4-char hex string, then endian-switched)
        // 5. Result: frame_size + '020000' + size + checksum + '0065' + png_data

        // Calculate PNG data size in bytes
        uint32_t png_size = pngData.size();

        // Convert to 4-byte hex string (8 hex chars = 4 bytes)
        char size_hex[9];
        snprintf(size_hex, sizeof(size_hex), "%08x", png_size);

        // Convert hex string to bytes and switch endian
        std::vector<uint8_t> png_size_bytes;
        for (int i = 6; i >= 0; i -= 2) {
            char byte_str[3] = {size_hex[i], size_hex[i+1], '\0'};
            png_size_bytes.push_back((uint8_t)strtol(byte_str, nullptr, 16));
        }

        // Calculate CRC32 of PNG data
        std::vector<uint8_t> crc_bytes = Helpers::calculateCRC32Bytes(pngData);

        // Build inner payload for frame size calculation: "FFFF020000" + size + checksum + "0065" + png_data
        // This is used ONLY to calculate the frame size
        uint32_t inner_size = 5 + 4 + 4 + 2 + pngData.size();  // FFFF020000 + size + checksum + 0065 + png_data

        // Convert to 2-byte hex string (4 hex chars = 2 bytes) and switch endian
        char frame_size_hex[5];
        snprintf(frame_size_hex, sizeof(frame_size_hex), "%04x", inner_size);

        std::vector<uint8_t> frame_size_bytes;
        for (int i = 2; i >= 0; i -= 2) {
            char byte_str[3] = {frame_size_hex[i], frame_size_hex[i+1], '\0'};
            frame_size_bytes.push_back((uint8_t)strtol(byte_str, nullptr, 16));
        }

        // Build final result: frame_size + "020000" + size + checksum + "0065" + png_data
        std::vector<uint8_t> result;
        result.insert(result.end(), frame_size_bytes.begin(), frame_size_bytes.end());
        result.push_back(0x02);
        result.push_back(0x00);
        result.push_back(0x00);
        result.insert(result.end(), png_size_bytes.begin(), png_size_bytes.end());
        result.insert(result.end(), crc_bytes.begin(), crc_bytes.end());
        result.push_back(0x00);
        result.push_back(0x65);
        result.insert(result.end(), pngData.begin(), pngData.end());

        return result;
    }

    std::vector<uint8_t> sendAnimation(const std::vector<uint8_t>& gifData) {
        if (gifData.empty()) {
            throw std::invalid_argument("GIF data cannot be empty");
        }

        // Python implementation:
        // checksum = CRC32_checksum(gif_hex)
        // size = get_frame_size(gif_hex, 8)
        // return bytes.fromhex(f"{get_frame_size('FFFF030000' + size + checksum + '0201' + gif_hex, 4)}030000{size}{checksum}0201{gif_hex}")

        // This means:
        // 1. Calculate size of GIF data (as 8-char hex string, then endian-switched)
        // 2. Calculate CRC32 of GIF data
        // 3. Build inner: 'FFFF030000' + size + checksum + '0201' + gif_hex
        // 4. Calculate frame_size of inner (as 4-char hex string, then endian-switched)
        // 5. Result: frame_size + '030000' + size + checksum + '0201' + gif_data

        // Calculate GIF data size in bytes
        uint32_t gif_size = gifData.size();

        // Convert to 4-byte hex string (8 hex chars = 4 bytes)
        char size_hex[9];
        snprintf(size_hex, sizeof(size_hex), "%08x", gif_size);

        // Convert hex string to bytes and switch endian
        std::vector<uint8_t> gif_size_bytes;
        for (int i = 6; i >= 0; i -= 2) {
            char byte_str[3] = {size_hex[i], size_hex[i+1], '\0'};
            gif_size_bytes.push_back((uint8_t)strtol(byte_str, nullptr, 16));
        }

        // Calculate CRC32 of GIF data
        std::vector<uint8_t> crc_bytes = Helpers::calculateCRC32Bytes(gifData);

        // Build inner payload for frame size calculation: "FFFF030000" + size + checksum + "0201" + gif_data
        // This is used ONLY to calculate the frame size
        uint32_t inner_size = 5 + 4 + 4 + 2 + gifData.size();  // FFFF030000 + size + checksum + 0201 + gif_data

        // Convert to 2-byte hex string (4 hex chars = 2 bytes) and switch endian
        char frame_size_hex[5];
        snprintf(frame_size_hex, sizeof(frame_size_hex), "%04x", inner_size);

        std::vector<uint8_t> frame_size_bytes;
        for (int i = 2; i >= 0; i -= 2) {
            char byte_str[3] = {frame_size_hex[i], frame_size_hex[i+1], '\0'};
            frame_size_bytes.push_back((uint8_t)strtol(byte_str, nullptr, 16));
        }

        // Build final result: frame_size + "030000" + size + checksum + "0201" + gif_data
        std::vector<uint8_t> result;
        result.insert(result.end(), frame_size_bytes.begin(), frame_size_bytes.end());
        result.push_back(0x03);
        result.push_back(0x00);
        result.push_back(0x00);
        result.insert(result.end(), gif_size_bytes.begin(), gif_size_bytes.end());
        result.insert(result.end(), crc_bytes.begin(), crc_bytes.end());
        result.push_back(0x02);
        result.push_back(0x01);
        result.insert(result.end(), gifData.begin(), gifData.end());

        return result;
    }

}
