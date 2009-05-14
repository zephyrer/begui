/* 
// Copyright 2007 Alexandros Panagopoulos
//
// This software is distributed under the terms of the GNU Lesser General Public Licence
//
// This file is part of BeGUI library.
//
//    BeGUI is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    BeGUI is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with BeGUI.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ScrollBar.h"
#include "ResourceManager.h"
#include "util.h"

using namespace begui;

ScrollBar::ScrollBar() : m_scrollDir(SCROLL_VERTICAL),
	m_minPos(0), m_maxPos(1), m_curPos(0),
	m_sliderDragStart(-1),
	m_percVisible(-1),
	m_sliderLen(20),
	m_barSize(18)
{
}

ScrollBar::~ScrollBar()
{
}

void ScrollBar::create(int x, int y, int length, ScrollDir dir, double minPos, double maxPos,
						const std::string &style_name)
{
	// set the dimensions of the component
	m_left = x;
	m_top = y;

	// set properties
	m_scrollDir = dir;
	m_minPos = minPos;
	m_maxPos = maxPos;
	m_curPos = minPos;

	// get the style of the scrollbar
	ResourceManager::Style barstyle = ResourceManager::inst()->getClassDef("ScrollBar").style(style_name);
	m_barBg = ResourceManager::inst()->loadImage(barstyle.get_img("bg"));
	
	if (dir == ScrollBar::SCROLL_HORIZONTAL)
	{
		m_barSize = m_barBg.m_height;
		m_right = x+length;
		m_bottom = y+m_barSize;
	}
	else
	{
		m_barSize = m_barBg.m_width;
		m_right = x+m_barSize;
		m_bottom = y+length;
	}

	// create the buttons
	if (dir == ScrollBar::SCROLL_HORIZONTAL) {
		ResourceManager::Style bstyle = ResourceManager::inst()->getClassDef("Button").style("scroller_btn_left");
		m_incBtn.create(-bstyle.get_i("padding_left"), -bstyle.get_i("padding_top"), 
						"", 101, makeFunctor(*this, &ScrollBar::handleClick), "scroller_btn_left");

		bstyle = ResourceManager::inst()->getClassDef("Button").style("scroller_btn_right");
		m_decBtn.create(m_right+bstyle.get_i("padding_right")-bstyle.get_i("default_width"), 
						-bstyle.get_i("padding_top"), 
						"", 101, makeFunctor(*this, &ScrollBar::handleClick), "scroller_btn_right");

		m_slider.create(0, 0, "", 103, Functor1<int>(), "scroller_slider");
	}
	else {
		m_incBtn.create(0, 0, "", 101, makeFunctor(*this, &ScrollBar::handleClick), "scroller_btn_down");
		Rect<int> actBorders = m_incBtn.getActiveBorders();
		m_incBtn.setPos(-actBorders.left, getHeight()-m_incBtn.getActiveArea().getHeight());
		
		
		m_decBtn.create(0, 0, "", 102, makeFunctor(*this, &ScrollBar::handleClick), "scroller_btn_up");
		actBorders = m_decBtn.getActiveBorders();
		m_decBtn.setPos(-actBorders.left, -actBorders.top);
		
		m_slider.create(0, 0, "", 103, Functor1<int>(), "scroller_slider");

		int sl_w = m_slider.getActiveArea().getWidth();
		int btn1_w = m_decBtn.getActiveArea().getWidth();
		int btn2_w = m_incBtn.getActiveArea().getWidth();
		int center_x = getWidth()/2;
		m_slider.setPos(center_x - sl_w/2, m_slider.getTop());
		m_decBtn.setPos(center_x - btn1_w/2, m_decBtn.getTop());
		m_incBtn.setPos(center_x - btn2_w/2, m_incBtn.getTop());
	}
	addComponent(&m_slider);
	addComponent(&m_incBtn);
	addComponent(&m_decBtn);
}

void ScrollBar::onUpdate()
{
	// update the position and size of the slider
	int runArea = 0;
	if (m_scrollDir == ScrollBar::SCROLL_VERTICAL) {
		runArea = m_incBtn.getActiveArea().top - m_decBtn.getActiveArea().bottom;
	}
	else {
		runArea = m_incBtn.getActiveArea().left - m_decBtn.getActiveArea().right;
	}

	if (m_percVisible > 1)
		m_percVisible = 1;
	if (m_percVisible > 0)
		m_sliderLen = (int)(runArea * m_percVisible);
	else
		m_sliderLen = (int)((runArea > 20) ? 20 : 0.8*runArea);

	//m_sliderLen += m_sliderOffs1-m_sliderOffs2;

	Rect<int> slActBorders = m_slider.getActiveBorders();
	if (m_scrollDir == ScrollBar::SCROLL_VERTICAL) {
		m_slider.setPos(m_slider.getLeft(), 
			m_decBtn.getActiveArea().bottom -slActBorders.top + 
			(int)((runArea - m_sliderLen)*(m_curPos-m_minPos)/(m_maxPos-m_minPos)) );

		m_slider.setSize(m_slider.getWidth(), (m_sliderLen - slActBorders.top + slActBorders.bottom));
	}
	else {
	}

	Container::onUpdate();
}

void ScrollBar::onRender()
{
	//Component::drawImage(m_barBg, (m_incBtn.getWidth() - m_barBg.m_width)/2 + m_incBtn.getLeft()+1 , 
	//				 m_incBtn.getBottom() - m_incBtn.getHeight()/2, 
	//				 m_barBg.m_width, getHeight());
	if (m_scrollDir == ScrollBar::SCROLL_VERTICAL) {
		int center_x = getWidth()/2;
		Component::drawImage(m_barBg, 
			center_x - m_barBg.m_width/2, 
			m_decBtn.getActiveArea().bottom-3, 
			0, 
			m_incBtn.getActiveArea().top-m_decBtn.getActiveArea().bottom+6);
	}
}
	
bool ScrollBar::onMouseDown(int x, int y, int button)
{
/*	int w = SCROLL_WIDTH-2;
	int h = SCROLL_WIDTH-2;

	x-=m_left;
	y-=m_top;

	double sliderPos = (m_curPos-m_minPos)/(m_maxPos - m_minPos);
	
	// adding (m_maxPos - m_minPos)*m_percVisible to m_curPos would
	// take us to the next page of content. Use that.
	double pageStep = (m_maxPos - m_minPos)/5;
	if (m_percVisible > 0)
		pageStep = (m_maxPos - m_minPos)*m_percVisible;

	if (m_scrollDir == ScrollBar::SCROLL_VERTICAL)
	{
		if (x <= 0 || x > w)
			return;

		if (y > 0 && y < h)
		{
			// up arrow clicked
			m_curPos -= pageStep/STEPS_PER_PAGE;
		}
		else if (y > getHeight()-h-2 && y < getHeight()-1)
		{
			// down arrow clicked
			m_curPos += pageStep/STEPS_PER_PAGE;
		}
		else if (y >= h+2 + (getHeight()-2*h-4-m_sliderLen)*sliderPos &&
			y <= h+2 + (getHeight()-2*h-4-m_sliderLen)*sliderPos + m_sliderLen)
		{
			m_sliderDragStart = y;
			m_sliderDragStartVal = m_curPos;
		}
		else
		{
			if (y > h+1 && y < getHeight()-h-1)
			{
				if (y < h+1+(getHeight()-2*h-2)*sliderPos)
					m_curPos -= pageStep;
				else
					m_curPos += pageStep;
			}
		}
	}
	else if (m_scrollDir == ScrollBar::SCROLL_HORIZONTAL)
	{
		if (y <= 0 || y > h)
			return;

		if (x > 0 && x < w)
		{
			// left arrow clicked
			m_curPos -= pageStep/STEPS_PER_PAGE;
		}
		else if (x > getWidth()-w-2 && x < getWidth()-1)
		{
			// right arrow clicked
			m_curPos += pageStep/STEPS_PER_PAGE;
		}
		else if (x >= w+2 + (getWidth()-2*w-4-m_sliderLen)*sliderPos &&
			x <= w+2 + (getWidth()-2*w-4-m_sliderLen)*sliderPos + m_sliderLen)
		{
			m_sliderDragStart = x;
			m_sliderDragStartVal = m_curPos;
		}
		else
		{
			if (x > w+1 && x < getWidth()-w-1)
			{
				if (x < w+1+(getWidth()-2*w-2)*sliderPos)
					m_curPos -= pageStep;
				else
					m_curPos += pageStep;
			}
		}
	}
	
	if (m_curPos < m_minPos)
		m_curPos = m_minPos;
	if (m_curPos > m_maxPos)
		m_curPos = m_maxPos;*/

	Vector2i lP = parentToLocal(Vector2i(x,y));
	if (m_slider.isPtInside(lP.x,lP.y))
	{
		m_sliderDragStart = y;
		m_sliderDragStartVal = m_curPos;
		
		m_slider.getMouseFocus();
		m_pActiveComponent = &m_slider;
		m_slider.onMouseDown(lP.x, lP.y, button);
	}
	else if (Container::onMouseDown(x,y,button))
	{
		return true;
	}
	else
	{
		// check if background area was clicked
		
		m_curPos = (m_maxPos - m_minPos)*(lP.y - m_decBtn.getBottom() - m_sliderLen/2) 
					/ (m_incBtn.getTop() - m_decBtn.getBottom() - m_sliderLen);
		if (m_curPos < m_minPos)
			m_curPos = m_minPos;
		if (m_curPos > m_maxPos)
			m_curPos = m_maxPos;
	}
	
	return false;
}

bool ScrollBar::onMouseMove(int x, int y, int prevx, int prevy)
{
	const int DRAG_MARGIN = 30;	// extra margins on the sides, where the drag is still considered valid

	if (m_sliderDragStart > -1)
	{
		if (x <= getLeft()-DRAG_MARGIN || x > getRight()+DRAG_MARGIN) {
			m_curPos = m_sliderDragStartVal;
			return false;
		}

		m_curPos = m_sliderDragStartVal + (m_maxPos - m_minPos)*(y - m_sliderDragStart)
				/(m_incBtn.getTop() - m_decBtn.getBottom() - m_sliderLen);
		if (m_curPos < m_minPos)
			m_curPos = m_minPos;
		if (m_curPos > m_maxPos)
			m_curPos = m_maxPos;

		return true;
	}

	return Container::onMouseMove(x,y,prevx,prevy);
}

bool ScrollBar::onMouseUp(int x, int y, int button)
{
	m_sliderDragStart = -1;

	return Container::onMouseUp(x,y,button);
}

void ScrollBar::onKeyDown(int key)
{
	// adding (m_maxPos - m_minPos)*m_percVisible to m_curPos would
	// take us to the next page of content. Use that.
	double pageStep = (m_maxPos - m_minPos)/5;
	if (m_percVisible > 0)
		pageStep = (m_maxPos - m_minPos)*m_percVisible;

	switch (key) {
	case KEY_UP:
		if (m_scrollDir == ScrollBar::SCROLL_VERTICAL)
		{
			m_curPos -= pageStep/STEPS_PER_PAGE;
			if (m_curPos < m_minPos)
				m_curPos = m_minPos;
		}
		break;
	case KEY_DOWN:
		if (m_scrollDir == ScrollBar::SCROLL_VERTICAL)
		{
			m_curPos += pageStep/STEPS_PER_PAGE;
			if (m_curPos > m_maxPos)
				m_curPos = m_maxPos;
		}
		break;
	case KEY_LEFT:
		if (m_scrollDir == ScrollBar::SCROLL_HORIZONTAL)
		{
			m_curPos -= pageStep/STEPS_PER_PAGE;
			if (m_curPos < m_minPos)
				m_curPos = m_minPos;
		}
		break;
	case KEY_RIGHT:
		if (m_scrollDir == ScrollBar::SCROLL_HORIZONTAL)
		{
			m_curPos += pageStep/STEPS_PER_PAGE;
			if (m_curPos > m_maxPos)
				m_curPos = m_maxPos;
		}
		break;
	case KEY_PAGEUP:
		m_curPos -= pageStep;
		if (m_curPos < m_minPos)
			m_curPos = m_minPos;
		break;
	case KEY_PAGEDOWN:
		m_curPos += pageStep;
		if (m_curPos > m_maxPos)
			m_curPos = m_maxPos;
		break;
	case KEY_HOME:
		m_curPos = m_minPos;
		break;
	case KEY_END:
		m_curPos = m_maxPos;
		break;
	}

	Container::onKeyDown(key);
}

void ScrollBar::onKeyUp(int key)
{
	Container::onKeyUp(key);
}

void ScrollBar::setBounds(double minPos, double maxPos, double percVisible)
{
	if (m_curPos < minPos) m_curPos = minPos;
	if (m_curPos > maxPos) m_curPos = maxPos;
	m_minPos = minPos;
	m_maxPos = maxPos;
	m_percVisible = percVisible;
}

void ScrollBar::handleClick(int id)
{
	double pageStep = (m_maxPos - m_minPos)/5;
	if (m_percVisible > 0)
		pageStep = (m_maxPos - m_minPos)*m_percVisible;

	switch (id) {
		case 102:
			m_curPos -= pageStep/STEPS_PER_PAGE;
			if (m_curPos < m_minPos)
				m_curPos = m_minPos;
			break;
		case 101:
			m_curPos += pageStep/STEPS_PER_PAGE;
			if (m_curPos > m_maxPos)
				m_curPos = m_maxPos;
			break;
	}
}