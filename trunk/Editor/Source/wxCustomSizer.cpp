

#include "../Header/wxCustomSizer.h"

wxCustomSizer::wxCustomSizer(wxWindow *window)
{
	mWindow = window;
	mCurrentLinePosition = 0.0f;
};

void wxCustomSizer::Reset()
{
	mCurrentLinePosition = 0.0f;
};

void wxCustomSizer::newLine(float distance_in_percent)
{
	mCurrentLinePosition += distance_in_percent * 0.01 * mWindow->GetSize().GetHeight();
};

void wxCustomSizer::Add(wxWindow *widget, float percent, bool xCenter, bool yCenter)
{
	widget->Show(true);

	float xPos = 0.0f;
	xPos = mWindow->GetSize().GetWidth() * 0.01 * percent;
	if (xCenter == true) xPos -= widget->GetSize().GetWidth() / 2;

	float yPos = 0.0f;
	yPos = mCurrentLinePosition;
	if (yCenter == true) yPos -= widget->GetSize().GetHeight() / 2;

	widget->SetPosition(wxPoint(xPos, yPos));
};

void wxCustomSizer::Add(wxWindow *widget, float percentFrom, float percentTo, bool yCenter)
{
	widget->Show(true);

	float xPos = 0.0f;
	xPos = mWindow->GetSize().GetWidth() * 0.01 * percentFrom;

	float yPos = 0.0f;
	yPos = mCurrentLinePosition;
	if (yCenter == true) yPos -= widget->GetSize().GetHeight() / 2;

	widget->SetPosition(wxPoint(xPos, yPos));

	float xSize = mWindow->GetSize().GetWidth() * 0.01 * percentTo - xPos;

	widget->SetSize(wxSize(xSize, -1));
};