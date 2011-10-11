#ifndef ABSTRACTMODEL_H
#define ABSTRACTMODEL_H

#include "itkObject.h"
#include "SNAPCommon.h"
#include "SNAPEvents.h"
#include "EventBucket.h"
#include <set>
#include <string>

class IRISObservable;


/**
  \class AbstractModel
  \brief Parent class for all UI models

  This class provides basic common functionality for all UI models. This
  includes the event/update mechanism.
  */
class AbstractModel : public itk::Object
{
public:
  /*
  typedef AbstractModel                  Self;
  typedef itk::Object                    Superclass;
  typedef itk::SmartPointer<Self>        Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  itkTypeMacro(AbstractModel, itk::Object)

  itkNewMacro(Self)
  */

  irisITKObjectMacro(AbstractModel, itk::Object)


  /**
    Call this function to update the model based on events that have been
    fired since the last time the model was updated. This function simply
    checks if the event bucket is empty; if not, it calls the OnUpdate()
    protected virtual method. Subclassers should reimplement OnUpdate(). The
    event bucket is emptied at the end of this call.
    */
  void Update();

protected:

  AbstractModel();
  virtual ~AbstractModel();

  /**
    Helper class for AbstractModel used to rebroadcast events
    */
  class Rebroadcaster
  {
  public:
    Rebroadcaster(AbstractModel *model, const itk::EventObject &evt);
    virtual ~Rebroadcaster();

    void Broadcast(itk::Object *source, const itk::EventObject &evt);

  private:
    AbstractModel *m_Model;
    itk::EventObject *m_Event;
  };

  /**
    Listen to events of type srcEvent on the object src, and rebroadcast
    them as event trgEvent. In the process, record the srcEvent in the
    event bucket. This is the main mechanism for model updates. The model
    listens to events occurring upstream. When an event occurs, the model
    only records the event and invokes its own event, to which the view
    objects downstream are listening. It is then the view's responsibility
    to call the Update() function on the model. This function checks what
    events are in the event bucket, and processes them in an orderly fashion.
    */
  void Rebroadcast(
      itk::Object *src, const itk::EventObject &srcEvent,
      const itk::EventObject &trgEvent);

  /**
    Rebroadcast is also defined for Property objects.
    */
  void Rebroadcast(IRISObservable &property, const itk::EventObject &trgEvent);


  /**
    This is the method called by Update() if there are events in the
    event bucket
    */
  virtual void OnUpdate() {}

  // List of rebroadcasters
  std::list<Rebroadcaster *> m_Rebroadcast;

  // Bucket that stores events fired since last call to Update()
  EventBucket *m_EventBucket;
};

#endif // ABSTRACTMODEL_H