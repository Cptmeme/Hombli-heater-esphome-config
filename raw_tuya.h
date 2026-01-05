#include "esphome.h"

// A dumb parser that just reads Tuya packets and extracts data
// No handshakes, no state machines, no errors.
class CustomTuyaParser : public Component, public UARTDevice {
 public:
  CustomTuyaParser(UARTComponent *parent) : UARTDevice(parent) {}

  // Sensors to publish data to Home Assistant
  TextSensor *room_temp_sensor = new TextSensor();
  TextSensor *target_temp_sensor = new TextSensor();
  BinarySensor *power_sensor = new BinarySensor();

  void setup() override {
    // nothing to setup
  }

  void loop() override {
    while (available()) {
      uint8_t c = read();
      buffer.push_back(c);
      
      // 1. Check Header (55 AA)
      if (buffer.size() >= 1 && buffer[0] != 0x55) {
        buffer.erase(buffer.begin());
        continue;
      }
      if (buffer.size() >= 2 && buffer[1] != 0xAA) {
        buffer.erase(buffer.begin()); // keep 0x55 if it was second
        continue;
      }

      // 2. Wait for Length Byte (at index 5)
      // Packet: Head(2) + Ver(1) + Cmd(1) + Len(2)
      if (buffer.size() > 6) {
        int len = (buffer[4] << 8) | buffer[5];
        int total_packet_len = 6 + len + 1; // Header(6) + Data(len) + Checksum(1)

        // 3. Process if we have the full packet
        if (buffer.size() >= total_packet_len) {
          std::vector<uint8_t> packet(buffer.begin(), buffer.begin() + total_packet_len);
          process_packet(packet);
          buffer.erase(buffer.begin(), buffer.begin() + total_packet_len);
        }
      }
    }
  }

  void process_packet(std::vector<uint8_t> packet) {
    // We only care about Command 0x07 (Status Report)
    if (packet[3] != 0x07) return;

    int pos = 6; // Data starts after Header+Ver+Cmd+Len
    int data_end = packet.size() - 1; // Exclude checksum

    while (pos < data_end) {
      uint8_t dp_id = packet[pos];
      // uint8_t dp_type = packet[pos + 1]; // Unused
      int dp_len = (packet[pos + 2] << 8) | packet[pos + 3];
      
      if (pos + 4 + dp_len > data_end) break; // Safety break

      // Extract Value (4 bytes for Int/Enum, 1 byte for Bool)
      int value = 0;
      if (dp_len == 1) {
        value = packet[pos + 4];
      } else if (dp_len == 4) {
        value = (packet[pos + 4] << 24) | (packet[pos + 5] << 16) | (packet[pos + 6] << 8) | packet[pos + 7];
      }

      // --- MAPPING ---
      // DP 1 = Power
      if (dp_id == 1) {
        power_sensor->publish_state(value == 1);
      }
      // DP 2 = Target Temp
      if (dp_id == 2) {
        target_temp_sensor->publish_state(to_string(value));
      }
      // DP 3 = Room Temp
      if (dp_id == 3) {
        room_temp_sensor->publish_state(to_string(value));
      }

      pos += 4 + dp_len;
    }
  }

  std::vector<uint8_t> buffer;
};
