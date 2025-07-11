
import sys
import struct

def stm32_crc32_hw_style(data: bytes) -> int:
    poly = 0x04C11DB7
    crc = 0xFFFFFFFF

    i = 0
    while i + 4 <= len(data):
        # 리틀엔디안으로 읽기
        word = data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) | (data[i + 3] << 24)
        crc ^= word
        for _ in range(32):
            crc = ((crc << 1) ^ poly) & 0xFFFFFFFF if (crc & 0x80000000) else (crc << 1) & 0xFFFFFFFF
        i += 4

    remaining = len(data) - i
    if remaining:
        tail = data[i:] + b'\x00' * (4 - remaining)
        word = tail[0] | (tail[1] << 8) | (tail[2] << 16) | (tail[3] << 24)
        crc ^= word
        for _ in range(32):
            crc = ((crc << 1) ^ poly) & 0xFFFFFFFF if (crc & 0x80000000) else (crc << 1) & 0xFFFFFFFF

    return crc



if __name__ == "__main__":
    bin_path = sys.argv[1]
    print(f"[INFO] Injecting into BIN file: {bin_path}")
    
    # === Tag 구조 상수 ===
    FIRMWARE_TAG_OFFSET = 0x0000         # firmware_tag 위치
    SIZE_OFFSET_IN_TAG  = 0              # size 필드 offset (0x00)
    CRC_OFFSET_IN_TAG   = 4              # crc 필드 offset (0x04)
    CRC_REGION_START    = 0x0400         # CRC 계산 시작 위치

    # === BIN 파일 로드 및 수정 ===
    with open(bin_path, 'rb') as f:
        bin_data = bytearray(f.read())  # bytearray로 읽어야 수정 가능

    file_size = len(bin_data)
    CRC_REGION_SIZE = file_size - CRC_REGION_START
    print(f"[INFO] CRC region size: {CRC_REGION_SIZE} bytes")

    # === 1. CRC 계산 전, 기존 CRC 값 무효화 (0xFFFFFFFF로 초기화) ===
    bin_data[FIRMWARE_TAG_OFFSET + CRC_OFFSET_IN_TAG : FIRMWARE_TAG_OFFSET + CRC_OFFSET_IN_TAG + 4] = b'\xFF\xFF\xFF\xFF'

    # === 2. CRC 계산 ===
    crc_data = bin_data[CRC_REGION_START:CRC_REGION_START + CRC_REGION_SIZE]
    crc = stm32_crc32_hw_style(crc_data)
    print(f"[INFO] Calculated STM32-style CRC: 0x{crc:08X}")

    # === 3. size 값 입력 ===
    bin_data[FIRMWARE_TAG_OFFSET + SIZE_OFFSET_IN_TAG : FIRMWARE_TAG_OFFSET + SIZE_OFFSET_IN_TAG + 4] = struct.pack('<I', CRC_REGION_SIZE )


    # === 4. CRC 값 입력 ===
    bin_data[FIRMWARE_TAG_OFFSET + CRC_OFFSET_IN_TAG : FIRMWARE_TAG_OFFSET + CRC_OFFSET_IN_TAG + 4] = struct.pack('<I', crc)

    # === 5. 전체 데이터를 다시 파일에 기록 ===
    with open(bin_path, 'wb') as f:
        f.write(bin_data)

    print("[INFO] Firmware tag updated successfully with CRC and size.")


    data = (0x20050000).to_bytes(4, byteorder='little')
    crc = stm32_crc32_hw_style(data)
    print(f"CRC = 0x{crc:08X}")