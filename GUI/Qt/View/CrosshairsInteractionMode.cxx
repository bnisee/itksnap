/*=========================================================================

  Program:   ITK-SNAP
  Module:    $RCSfile: Filename.cxx,v $
  Language:  C++
  Date:      $Date: 2010/10/18 11:25:44 $
  Version:   $Revision: 1.12 $
  Copyright (c) 2011 Paul A. Yushkevich

  This file is part of ITK-SNAP

  ITK-SNAP is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

=========================================================================*/

#include "CrosshairsInteractionMode.h"
#include "GenericSliceView.h"
#include "OrthogonalSliceCursorNavigationModel.h"
#include "CrosshairsRenderer.h"

CrosshairsInteractionMode::CrosshairsInteractionMode(QWidget *parent) :
    QtInteractionDelegateWidget(parent)
{
  SetMouseButtonBehaviorToCrosshairsMode();
  setAttribute(Qt::WA_AcceptTouchEvents, true);
  m_WheelEventTarget = NULL;
}

void CrosshairsInteractionMode
::SetMouseButtonBehaviorToCrosshairsMode()
{
  m_BtnCursor = Qt::LeftButton;
  m_BtnZoom = Qt::RightButton;
  m_BtnPan = Qt::MiddleButton;
}

void CrosshairsInteractionMode
::SetMouseButtonBehaviorToZoomPanMode()
{
  m_BtnCursor = Qt::MiddleButton;
  m_BtnZoom = Qt::RightButton;
  m_BtnPan = Qt::LeftButton;
}

void CrosshairsInteractionMode
::SetModel(OrthogonalSliceCursorNavigationModel *model)
{
  m_Model = model;
  m_Renderer.SetModel(model);
}

void CrosshairsInteractionMode::mousePressEvent(QMouseEvent *ev)
{
  // Use model to envoke event
  if(ev->button() == m_BtnCursor)
    {
    m_Model->UpdateCursor(Vector2f(m_XSpace[0], m_XSpace[1]));
    }
  else if(ev->button() == m_BtnZoom)
    {
    m_Model->BeginZoom();
    }
  else if(ev->button() == m_BtnPan)
    {
    m_Model->BeginPan();
    }
  // Eat this event
  ev->accept();
}

void CrosshairsInteractionMode::mouseMoveEvent(QMouseEvent *ev)
{
  Vector3d dx = m_XSpace - m_LastPressXSpace;

  if(m_LastPressButton == m_BtnCursor)
    {
    m_Model->UpdateCursor(Vector2f(m_XSpace[0], m_XSpace[1]));
    }
  else if(m_LastPressButton == m_BtnZoom)
    {    
    double scaleFactor = pow(1.02f, dx(1));
    m_Model->ProcessZoomGesture(scaleFactor);
    }
  else if(m_LastPressButton == m_BtnPan)
    {
    m_Model->ProcessPanGesture(Vector2f(dx(0), dx(1)));
    }

  // Eat this event
  ev->accept();
}


void CrosshairsInteractionMode::mouseReleaseEvent(QMouseEvent *ev)
{

  if(ev->button() == m_BtnCursor)
    {
    m_Model->UpdateCursor(Vector2f(m_XSpace[0], m_XSpace[1]));
    }
  else if(ev->button() == m_BtnZoom)
    {
    m_Model->EndZoom();
    }
  else if(ev->button() == m_BtnPan)
    {
    m_Model->EndPan();
    }
  // Eat this event
  ev->accept();
}

bool CrosshairsInteractionMode::gestureEvent(QGestureEvent *ev)
{
  // Get the pinch gesture if there is one
  if(QPinchGesture *pinch =
      static_cast<QPinchGesture *>(ev->gesture(Qt::PinchGesture)))
    {
    if(pinch->state() == Qt::GestureStarted)
      {
      m_Model->BeginZoom();
      }
    else if(pinch->state() == Qt::GestureUpdated)
      {
      m_Model->ProcessZoomGesture(pinch->scaleFactor());
      }
    else if(pinch->state() == Qt::GestureFinished)
      {
      m_Model->ProcessZoomGesture(pinch->scaleFactor());
      m_Model->EndZoom();
      }
    ev->accept();
    return true;
    }

  // Get the pan event
  if(QPanGesture *pan =
      static_cast<QPanGesture *>(ev->gesture(Qt::PanGesture)))
    {
    if(pan->state() == Qt::GestureStarted)
      {
      m_Model->BeginPan();
      }
    else if(pan->state() == Qt::GestureUpdated)
      {
      m_Model->ProcessPanGesture(Vector2f(pan->delta().x(), pan->delta().y()));
      }
    else if(pan->state() == Qt::GestureFinished)
      {
      m_Model->ProcessPanGesture(Vector2f(pan->delta().x(), pan->delta().y()));
      m_Model->EndPan();
      }
    ev->accept();
    return true;
    }

  // Get the swipe event
  if(QSwipeGesture *swipe =
      static_cast<QSwipeGesture *>(ev->gesture(Qt::SwipeGesture)))
    {
    if(swipe->verticalDirection() == QSwipeGesture::Down)
      {
      m_Model->ProcessScrollGesture(1);
      }
    else if(swipe->verticalDirection() == QSwipeGesture::Up)
      {
      m_Model->ProcessScrollGesture(-1);
      }
    ev->accept();
    return true;
    }
  return false;
}

void CrosshairsInteractionMode::enterEvent(QEvent *)
{
  // Respond to standard gestures
  this->grabGesture(Qt::PinchGesture);
  this->grabGesture(Qt::PanGesture);
  this->grabGesture(Qt::SwipeGesture);
}

void CrosshairsInteractionMode::leaveEvent(QEvent *)
{
  // Stop responding to gestures
  // this->ungrabGesture(Qt::PinchGesture);
}

void CrosshairsInteractionMode::wheelEvent(QWheelEvent *event)
{
  if(m_WheelEventTarget)
    {
    QWheelEvent *evnew = new QWheelEvent(
          event->pos(), event->globalPos(), event->delta(),
          event->buttons(), event->modifiers(),
          event->orientation());
    QCoreApplication::postEvent(m_WheelEventTarget, evnew);
    event->accept();
    }
    /*
  int numDegrees = event->delta() / 8;
  int numSteps = numDegrees / 15;
  std::cout << "Wheel event: " << event->delta() << std::endl;

  // Scroll
  m_Model->ProcessScrollGesture(numSteps);
  */
}

bool CrosshairsInteractionMode::event(QEvent *ev)
{
  return QtInteractionDelegateWidget::event(ev);
}

void CrosshairsInteractionMode::SetWheelEventTargetWidget(QWidget *w)
{
  m_WheelEventTarget = w;
}
