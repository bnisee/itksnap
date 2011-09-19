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

#ifndef GENERICSLICEMODEL_H
#define GENERICSLICEMODEL_H

#include <SNAPCommon.h>
#include <ImageCoordinateTransform.h>
#include <OpenGLSliceTexture.h>
#include <SNAPEvents.h>


class GlobalUIModel;
class IRISApplication;
class GenericImageData;
class ImageWrapperBase;
class GenericSliceModel;

// An event fired when the geometry of the slice view changes
itkEventMacro(SliceModelImageDimensionsChangeEvent, IRISEvent)
itkEventMacro(SliceModelGeometryChangeEvent, IRISEvent)



/**
  \class GenericSliceModel
  \brief Describes the state of the slice panel showing an orthogonal
  projection of a dataset in ITK-SNAP

  This class holds the state of the slice viewer widget. It contains
  information about the slice currently being shown, the mapping of
  coordinates between slice space and image space, and other bits of
  information. Ideally, you should be able to store and retrieve the
  state of this object between sessions.

  This class is meant to be used with arbitrary GUI environments. It is
  unaware of Qt, FLTK, etc.
*/
class GenericSliceModel : public itk::Object
{

public:

  /**
   * Constructor: takes global UI model and slice ID as input
   */
  GenericSliceModel(GlobalUIModel *, int);

  ~GenericSliceModel();

  /**
   * Initialize the slice view with image data
   */
  void InitializeSlice(GenericImageData *data);

  /**
   * Reset the view parameters of the window (zoom, view position) to
   * defaults
   */
  virtual void ResetViewToFit();

  /**
   * Map a point in window coordinates to a point in slice coordinates
   * (Window coordinates are the ones stored in FLTKEvent.xSpace)
   */
  Vector3f MapWindowToSlice(const Vector2f &xWindow);

  /**
   * Map an offset in window coordinates to an offset in slice coordinates
   */
  Vector3f MapWindowOffsetToSliceOffset(const Vector2f &xWindowOffset);

  /**
   * Map a point in slice coordinates to a point in window coordinates
   * (Window coordinates are the ones stored in FLTKEvent.xSpace)
   */
  Vector2f MapSliceToWindow(const Vector3f &xSlice);

  /**
   * Map a point in slice coordinates to a point in PHYISCAL window coordinates
   */
  Vector2f MapSliceToPhysicalWindow(const Vector3f &xSlice);

  /**
   * Map a point in slice coordinates to a point in the image coordinates
   */
  Vector3f MapSliceToImage(const Vector3f &xSlice);

  /**
   * Map a point in image coordinates to slice coordinates
   */
  Vector3f MapImageToSlice(const Vector3f &xImage);

  /**
   * Get the cursor position in slice coordinates, shifted to the center
   * of the voxel
   */
  Vector3f GetCursorPositionInSliceCoordinates();

  /**
   * Get the slice index for this window
   */
  unsigned int GetSliceIndex();

  /**
   * Set the index of the slice in the current view. This method will
   * update the cursor in the IRISApplication object
   */
  void UpdateSliceIndex(unsigned int newIndex);

  /**
   * Get the number of slices available in this view
   */
  unsigned int GetNumberOfSlices() const;

  /** Return the image axis along which this window shows slices */
  size_t GetSliceDirectionInImageSpace()
    { return m_ImageAxes[2]; }

  /** Reset the view position to center of the image */
  void ResetViewPosition ();

  /** Return the offset from the center of the viewport to the cursor position
   * in slice units (#voxels * spacing). This is used to synchronize panning
   * across SNAP sessions */
  Vector2f GetViewPositionRelativeToCursor();

  /** Set the offset from the center of the viewport to the cursor position */
  void SetViewPositionRelativeToCursor(Vector2f offset);

  /** Set the zoom factor (number of pixels on the screen per millimeter in
   * image space */
  irisSetWithEventMacro(ViewZoom, float, SliceModelGeometryChangeEvent)

  /** Get the zoom factor (number of pixels on the screen per millimeter in
   * image space */
  irisGetMacro(ViewZoom,float)

  /** Compute the optimal zoom (best fit) */
  irisGetMacro(OptimalZoom,float)

  /** Set the zoom management flag */
  irisSetMacro(ManagedZoom,bool)

  /** Set the view position **/
  void SetViewPosition(Vector2f);

  /** Get the view position **/
  irisGetMacro(ViewPosition, Vector2f)

  /** Get the slice spacing in the display space orientation */
  irisGetMacro(SliceSpacing,Vector3f)

  /** Get the slice spacing in the display space orientation */
  irisGetMacro(SliceSize,Vector3i)

  /** The id (slice direction) of this slice model */
  irisGetMacro(Id, int)

  /** Get the physical size of the window (updated from widget via events) */
  irisGetMacro(Size, Vector2ui)

  /** Has the slice model been initialized with image data? */
  irisIsMacro(SliceInitialized)

  irisGetMacro(ParentUI, GlobalUIModel *)
  irisGetMacro(Driver, IRISApplication *)

  irisGetMacro(ImageData, GenericImageData *)

  irisGetMacro(ThumbnailPosition, Vector2i)
  irisGetMacro(ThumbnailSize, Vector2i)
  irisGetMacro(ThumbnailZoom, float)

  /** Compute the canvas size needed to display slice at current zoom factor */
  Vector2i GetOptimalCanvasSize();

  /** This is a callback that the view must call whenever its size changes */
  void onViewResize(int w, int h);

  /** This method computes the thumbnail properties (size, zoom) */
  void ComputeThumbnailProperties();

  // Check whether the thumbnail should be drawn or not
  bool IsThumbnailOn();

  /** Callback for the event that the source data has been updated */
  void OnSourceDataUpdate();

protected:

  // Parent (where the global UI state is stored)
  GlobalUIModel *m_ParentUI;

  // Top-level logic object
  IRISApplication *m_Driver;

  // Pointer to the image data
  GenericImageData *m_ImageData;

  // Window id, equal to the direction in display space along which the
  // window shows slices
  int m_Id;

  // The index of the image space axes corresponding to the u,v,w of the
  // window (computed by applying a transform to the DisplayAxes)
  int m_ImageAxes[3];

  // The transform from image coordinates to display coordinates
  ImageCoordinateTransform m_ImageToDisplayTransform;

  // The transform from display coordinates to image coordinates
  ImageCoordinateTransform m_DisplayToImageTransform;

  // The transform from display coordinates to patient coordinates
  ImageCoordinateTransform m_DisplayToAnatomyTransform;

  // Dimensions of the current slice (the third component is the size
  // of the image in the slice direction)
  Vector3i m_SliceSize;

  // Pixel dimensions for the slice.  (the third component is the pixel
  // width in the slice direction)
  Vector3f m_SliceSpacing;

  // Position of visible window in slice space coordinates
  Vector2f m_ViewPosition;

  // The view position where the slice wants to be
  Vector2f m_OptimalViewPosition;

  // Size of the current view widget (updated through signals)
  Vector2ui m_Size;

  // The number of screen pixels per mm of image
  float m_ViewZoom;

  // The zoom level at which the slice fits snugly into the window
  float m_OptimalZoom;

  // Flag indicating whether the window's zooming is managed externally
  // by the SliceWindowCoordinator
  bool m_ManagedZoom;

  // The default screen margin (area into which we do not paint) at
  // least in default zoom
  unsigned int m_Margin;

  // A map between image layers and texture objects
  typedef std::map<ImageWrapperBase *, OpenGLSliceTexture *> TextureMap;

  // A dynamic association between various image layers and texture objects
  TextureMap m_Texture;

  // Internal method used by UpdateTextureMap()
  void AssociateTexture(
    ImageWrapperBase *iw, TextureMap &src, TextureMap &trg);

  // Update the texture map to mirror the current images in the model
  void UpdateTextureMap();

  // The position and size of the zoom thumbnail
  Vector2i m_ThumbnailPosition, m_ThumbnailSize;

  // The zoom level in the thumbnail
  double m_ThumbnailZoom;

  // State of the model (whether it's been initialized)
  bool m_SliceInitialized;

  // Computes the zoom that gives the best fit for the window
  void ComputeOptimalZoom();

  /** Access the next window in the slice pipeline */
  GenericSliceModel *GetNextSliceWindow();
};

#endif // GENERICSLICEMODEL_H
