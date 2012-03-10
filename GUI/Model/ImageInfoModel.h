#ifndef IMAGEINFOMODEL_H
#define IMAGEINFOMODEL_H

#include "AbstractLayerAssociatedModel.h"
#include "EditableNumericValueModel.h"
#include "GreyImageWrapper.h"

class ImageInfoLayerProperties
{
public:
  irisGetSetMacro(ObserverTag, unsigned long)

protected:

  // Whether or not we are already listening to events from this layer
  unsigned long m_ObserverTag;
};

typedef AbstractLayerAssociatedModel<
    ImageInfoLayerProperties,
    GreyImageWrapperBase> ImageInfoModelBase;

class ImageInfoModel : public ImageInfoModelBase
{
public:
  irisITKObjectMacro(ImageInfoModel, ImageInfoModelBase)

  // Implementation of virtual functions from parent class
  void RegisterWithLayer(GreyImageWrapperBase *layer);
  void UnRegisterFromLayer(GreyImageWrapperBase *layer);

  // PArent model assignment override
  virtual void SetParentModel(GlobalUIModel *parent);

  // Some model types
  typedef AbstractEditableNumericValueModel<Vector3d> RealVectorValueModel;
  typedef AbstractEditableNumericValueModel<Vector3ui> UIntVectorValueModel;

  // Access the individual models
  irisGetMacro(ImageDimensionsModel, UIntVectorValueModel *)
  irisGetMacro(ImageSpacingModel, RealVectorValueModel *)
  irisGetMacro(ImageOriginModel, RealVectorValueModel *)
  irisGetMacro(ImageItkCoordinatesModel, RealVectorValueModel *)
  irisGetMacro(ImageNiftiCoordinatesModel, RealVectorValueModel *)
  irisGetMacro(ImageVoxelCoordinatesModel, UIntVectorValueModel *)

protected:

  SmartPtr<RealVectorValueModel> m_ImageSpacingModel;
  SmartPtr<RealVectorValueModel> m_ImageOriginModel;
  SmartPtr<RealVectorValueModel> m_ImageItkCoordinatesModel;
  SmartPtr<RealVectorValueModel> m_ImageNiftiCoordinatesModel;
  SmartPtr<UIntVectorValueModel> m_ImageDimensionsModel;
  SmartPtr<UIntVectorValueModel> m_ImageVoxelCoordinatesModel;

  bool GetImageDimensionsValueAndRange(
      Vector3ui &value, NumericValueRange<Vector3ui> *range);

  bool GetImageOriginValueAndRange(
      Vector3d &value, NumericValueRange<Vector3d> *range);

  bool GetImageSpacingValueAndRange(
      Vector3d &value, NumericValueRange<Vector3d> *range);

  bool GetImageItkCoordinatesValueAndRange(
      Vector3d &value, NumericValueRange<Vector3d> *range);

  bool GetImageNiftiCoordinatesValueAndRange(
      Vector3d &value, NumericValueRange<Vector3d> *range);

  bool GetImageVoxelCoordinatesValueAndRange(
      Vector3ui &value, NumericValueRange<Vector3ui> *range);

  void SetImageVoxelCoordinates(Vector3ui value);



  ImageInfoModel();
  virtual ~ImageInfoModel() {}
};

#endif // IMAGEINFOMODEL_H
