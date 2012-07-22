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

#include "GlobalUIModel.h"

#include <IRISApplication.h>
#include <SNAPAppearanceSettings.h>
#include <GenericSliceModel.h>
#include <OrthogonalSliceCursorNavigationModel.h>
#include <PolygonDrawingModel.h>
#include <SliceWindowCoordinator.h>
#include <GenericImageData.h>
#include <GuidedNativeImageIO.h>
#include <ImageIODelegates.h>
#include <IntensityCurveModel.h>
#include <LayerSelectionModel.h>
#include <ColorMapModel.h>
#include <ImageInfoModel.h>
#include <Generic3DModel.h>
#include <LabelEditorModel.h>

#include <SNAPUIFlag.h>
#include <SNAPUIFlag.txx>

// Enable this model to be used with the flag engine
template class SNAPUIFlag<GlobalUIModel, UIState>;


GlobalUIModel::GlobalUIModel()
  : AbstractModel()
{
  // Create the appearance settings objects
  m_AppearanceSettings = new SNAPAppearanceSettings();

  // Create the IRIS application login
  m_Driver = IRISApplication::New();

  // Create the slice models
  for (unsigned int i = 0; i < 3; i++)
    {
    m_SliceModel[i] = GenericSliceModel::New();
    m_SliceModel[i]->Initialize(this, i);
    m_CursorNavigationModel[i] =
        OrthogonalSliceCursorNavigationModel::New();
    m_CursorNavigationModel[i]->SetParent(m_SliceModel[i]);

    m_PolygonDrawingModel[i] = PolygonDrawingModel::New();
    m_PolygonDrawingModel[i]->SetParent(m_SliceModel[i]);
    }

  // Connect them together with the coordinator
  m_SliceCoordinator = SliceWindowCoordinator::New();
  GenericSliceModel *ptr[3] =
    { m_SliceModel[0], m_SliceModel[1], m_SliceModel[2] };
  m_SliceCoordinator->RegisterSliceModels(ptr);

  // Intensity curve model
  m_IntensityCurveModel = IntensityCurveModel::New();
  m_IntensityCurveModel->SetParentModel(this);

  // Color map model
  m_ColorMapModel = ColorMapModel::New();
  m_ColorMapModel->SetParentModel(this);

  // Image info model
  m_ImageInfoModel = ImageInfoModel::New();
  m_ImageInfoModel->SetParentModel(this);

  // Layer selections
  m_LoadedLayersSelectionModel = LayerSelectionModel::New();
  m_LoadedLayersSelectionModel->SetParentModel(this);
  m_LoadedLayersSelectionModel->SetRoleFilter(
        LayerIterator::MAIN_ROLE | LayerIterator::OVERLAY_ROLE);

  // 3D model
  m_Model3D = Generic3DModel::New();
  m_Model3D->Initialize(this);

  // Label editor model
  m_LabelEditorModel = LabelEditorModel::New();
  m_LabelEditorModel->SetParentModel(this);

  // Initialize the properties
  m_ToolbarModeModel = NewSimpleConcreteProperty(CROSSHAIRS_MODE);

  // Set up the cursor position model
  m_CursorPositionModel = makeChildPropertyModel(
        this,
        &Self::GetCursorPositionValueAndRange,
        &Self::SetCursorPosition);

  // The model needs to rebroadcast cusror change events as value changes
  m_CursorPositionModel->Rebroadcast(
        this, CursorUpdateEvent(), ValueChangedEvent());

  // Create label information models. TODO: should we move this into a
  // separate model object?
  m_LabelUnderTheCursorIdModel = makeChildPropertyModel(
        this, &GlobalUIModel::GetLabelUnderTheCursorIdValue);

  // The models are hooked up to changes in label under the cursor
  m_LabelUnderTheCursorIdModel->Rebroadcast(
        this, LabelUnderCursorChangedEvent(), ValueChangedEvent());

  m_LabelUnderTheCursorTitleModel = makeChildPropertyModel(
        this, &GlobalUIModel::GetLabelUnderTheCursorTitleValue);

  // The models are hooked up to changes in label under the cursor
  m_LabelUnderTheCursorTitleModel->Rebroadcast(
        this, LabelUnderCursorChangedEvent(), ValueChangedEvent());

  // Listen to state changes from the slice coordinator
  Rebroadcast(m_SliceCoordinator, LinkedZoomUpdateEvent(), LinkedZoomUpdateEvent());
  Rebroadcast(m_SliceCoordinator, LinkedZoomUpdateEvent(), StateMachineChangeEvent());

  // Rebroadcast cursor change events
  Rebroadcast(m_Driver, CursorUpdateEvent(), CursorUpdateEvent());

  // Rebroadcast image layer change events
  Rebroadcast(m_Driver, LayerChangeEvent(), LayerChangeEvent());
  Rebroadcast(m_Driver, LayerChangeEvent(), StateMachineChangeEvent());

  // Rebroadcast toolbar model change events (TODO: needed?)
  Rebroadcast(m_ToolbarModeModel, ValueChangedEvent(), ToolbarModeChangeEvent());

  // All the events that result in the voxel under the cursor changing
  Rebroadcast(this, CursorUpdateEvent(), LabelUnderCursorChangedEvent());
  Rebroadcast(m_Driver->GetColorLabelTable(), SegmentationLabelChangeEvent(),
              LabelUnderCursorChangedEvent());
  Rebroadcast(m_Driver, SegmentationChangeEvent(), LabelUnderCursorChangedEvent());
}

GlobalUIModel::~GlobalUIModel()
{
  delete m_AppearanceSettings;
}

bool GlobalUIModel::CheckState(UIState state)
{
  // TODO: implement all the other cases
  switch(state)
    {
    case UIF_RGB_LOADED:
      return m_Driver->GetCurrentImageData()->IsRGBLoaded();
    case UIF_BASEIMG_LOADED:
      return m_Driver->GetCurrentImageData()->IsMainLoaded();
    case UIF_IRIS_ACTIVE:
      return true; // TODO: for now!
    case UIF_MESH_DIRTY:
      return false; // TODO:
    case UIF_MESH_ACTION_PENDING:
      break;
    case UIF_ROI_VALID:
      break;
    case UIF_LINKED_ZOOM:
      return m_SliceCoordinator->GetLinkedZoom();
    case UIF_UNDO_POSSIBLE:
      break;
    case UIF_REDO_POSSIBLE:
      break;
    case UIF_UNSAVED_CHANGES:
      break;
    case UIF_MESH_SAVEABLE:
      break;
    case UIF_GRAY_LOADED:
      return m_Driver->GetCurrentImageData()->IsMainLoaded();
    case UIF_OVERLAY_LOADED:
      return m_Driver->GetCurrentImageData()->IsOverlayLoaded();
    }

  return false;
}

void GlobalUIModel::LoadGrayImage(GuidedNativeImageIO *io)
{
  m_Driver->UnloadOverlays();
  m_Driver->UpdateIRISMainImage(io, IRISApplication::MAIN_SCALAR);
}

void GlobalUIModel
::LoadImageNonInteractive(const char *fname,
                          AbstractLoadImageDelegate &del,
                          IRISWarningList &wl)
{
  // Load the settings associated with this file
  Registry reg;
  m_Driver->GetSystemInterface()->FindRegistryAssociatedWithFile(fname, reg);

  // Get the folder dealing with grey image properties
  Registry &folder = reg.Folder("Files.Grey");

  // Create a native image IO object
  GuidedNativeImageIO io;

  // Load the header of the image
  io.ReadNativeImageHeader(fname, folder);

  // Validate the header
  del.ValidateHeader(&io, wl);

  // Read the image body
  io.ReadNativeImageData();

  // Validate the image data
  del.ValidateImage(&io, wl);

  // Put the image in the right place
  del.UpdateApplicationWithImage(&io);
}


SystemInterface * GlobalUIModel::GetSystemInterface() const
{
  return m_Driver->GetSystemInterface();
}

GlobalState * GlobalUIModel::GetGlobalState() const
{
  return m_Driver->GetGlobalState();
}

bool GlobalUIModel::GetLabelUnderTheCursorIdValue(LabelType &value)
{
  if(m_Driver->GetCurrentImageData()->IsSegmentationLoaded())
    {
    value = m_Driver->GetCurrentImageData()->GetSegmentation()
        ->GetVoxel(m_Driver->GetCursorPosition());
    return true;
    }
  return false;
}

bool GlobalUIModel::GetLabelUnderTheCursorTitleValue(std::string &value)
{
  if(m_Driver->GetCurrentImageData()->IsSegmentationLoaded())
    {
    LabelType label = m_Driver->GetCurrentImageData()->GetSegmentation()
        ->GetVoxel(m_Driver->GetCursorPosition());
    value = m_Driver->GetColorLabelTable()->GetColorLabel(label).GetLabel();
    return true;
    }
  return false;
}

bool GlobalUIModel::GetCursorPositionValueAndRange(
    Vector3ui &value, NumericValueRange<Vector3ui> *range)
{
  if(m_Driver->GetCurrentImageData()->IsMainLoaded())
    {
    value = m_Driver->GetCursorPosition();
    if(range)
      {
      range->Set(Vector3ui(0),
                 m_Driver->GetCurrentImageData()->GetMain()->GetSize() - 1u,
                 Vector3ui(1));
      }
    return true;
    }

  return false;
}

void GlobalUIModel::SetCursorPosition(Vector3ui value)
{
  m_Driver->SetCursorPosition(value);
}


