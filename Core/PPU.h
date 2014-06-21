#pragma once

#include "stdafx.h"
#include "MemoryManager.h"
#include "IVideoDevice.h"

enum PPURegisters
{
	Control = 0x00,
	Control2 = 0x01,
	Status = 0x02,
	SpriteAddr = 0x03,
	SpriteData = 0x04,
	ScrollOffsets = 0x05,
	VideoMemoryAddr = 0x06,
	VideoMemoryData = 0x07,
	SpriteDMA = 0x4014,
};

struct PPUControlFlags
{
	uint16_t NameTableAddr;
	bool VerticalWrite;
	uint16_t SpritePatternAddr;
	uint16_t BackgroundPatternAddr;
	bool LargeSprites;
	bool VBlank;

	bool Grayscale;
	bool BackgroundMask;
	bool SpriteMask;
	bool BackgroundEnabled;
	bool SpritesEnabled;
	bool IntensifyRed;
	bool IntensifyGreen;
	bool IntensifyBlue;
};

struct PPUStatusFlags
{
	bool SpriteOverflow;
	bool Sprite0Hit;
	bool VerticalBlank;
};

struct PPUState
{
	uint8_t Control;
	uint8_t Control2;
	uint8_t Status;
	uint32_t SpriteRamAddr;
	uint16_t VideoRamAddr;
	uint8_t XScroll;
	uint16_t TmpVideoRamAddr;
	bool WriteToggle;

   uint16_t HighBitShift;
   uint16_t LowBitShift;
};

struct TileInfo
{
	uint8_t LowByte;
	uint8_t HighByte;
	uint8_t PaletteOffset;
	bool HorizontalMirror;
};

class PPU : public IMemoryHandler
{
	private:
		static IVideoDevice *VideoDevice;

		MemoryManager *_memoryManager;

		PPUState _state;
		int32_t _scanline = 0;
		uint32_t _cycle = 0;
		uint32_t _frameCount = 0;
		uint64_t _cycleCount = 0;
		uint8_t _memoryReadBuffer = 0;

		uint8_t _paletteRAM[0x100];

		uint8_t _spriteRAM[0x100];
		uint8_t _secondarySpriteRAM[0x20];

		uint8_t *_outputBuffer;
		
		PPUControlFlags _flags;
		PPUStatusFlags _statusFlags;

		bool _doNotSetVBFlag = false;

		TileInfo _currentTile;
		TileInfo _nextTile;
		TileInfo _previousTile;

		int32_t _spriteX[8];
		TileInfo _spriteTiles[8];
		
		void UpdateStatusFlag();

		void UpdateFlags();
		bool CheckFlag(PPUControlFlags flag);

		bool IsRenderingEnabled();

		void IncVerticalScrolling();
		void IncHorizontalScrolling();
		uint16_t GetNameTableAddr();
		uint16_t GetAttributeAddr();

		void ProcessPreVBlankScanline();
		void ProcessPrerenderScanline();
		void ProcessVisibleScanline();

		void CopyOAMData();

		void BeginVBlank();
		void EndVBlank();

		uint8_t GetBGPaletteEntry(uint8_t paletteOffset, uint8_t pixel);
		uint8_t GetSpritePaletteEntry(uint8_t paletteOffset, uint8_t pixel);

		uint8_t ReadPaletteRAM(uint16_t addr);
		void WritePaletteRAM(uint16_t addr, uint8_t value);

		void LoadTileInfo();
		void LoadSpriteTileInfo(uint8_t spriteIndex);
		void ShiftTileRegisters();
		void InitializeShiftRegisters();
		void LoadNextTile();
		void DrawPixel();

		void CopyFrame();

		PPURegisters GetRegisterID(uint16_t addr)
		{
			if(addr == 0x4014) {
				return PPURegisters::SpriteDMA;
			} else {
				return (PPURegisters)(addr & 0x07);
			}
		}

	public:
		PPU(MemoryManager *memoryManager);
		~PPU();

		vector<std::array<uint16_t, 2>> GetRAMAddresses()
		{
			return{ { { 0x2000, 0x3FFF } }, { {0x4014, 0x4014 } } };
		}

		uint8_t ReadRAM(uint16_t addr);
		void WriteRAM(uint16_t addr, uint8_t value);

		void Exec();

		static void RegisterVideoDevice(IVideoDevice *videoDevice)
		{
			PPU::VideoDevice = videoDevice;
		}

		uint32_t GetFrameCount()
		{
			return _frameCount;
		}
};
