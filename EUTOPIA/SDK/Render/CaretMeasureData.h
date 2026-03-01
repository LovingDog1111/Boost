#pragma once

struct CaretMeasureData {
	float position;
	bool shouldRender;
	CaretMeasureData()
	{
		CaretMeasureData(0xFFFFFFFF, true);
	};

	CaretMeasureData(int position, bool singlelines)
	{
		this->position = position;
		this->shouldRender = singlelines;
	};
};