/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef otbMWImageIOFactory_h
#define otbMWImageIOFactory_h

#include "itkObjectFactoryBase.h"

namespace otb
{
/** \class MWImageIOFactory
 * \brief Creation d'un instance d'un objet LUMImageIO utilisant les object factory.
 *
 * \ingroup OTBIOMW
 */
class ITK_EXPORT MWImageIOFactory : public itk::ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef MWImageIOFactory              Self;
  typedef itk::ObjectFactoryBase        Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Class methods used to interface with the registered factories. */
  const char* GetITKSourceVersion(void) const ITK_OVERRIDE;
  const char* GetDescription(void) const ITK_OVERRIDE;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static MWImageIOFactory * FactoryNew() { return new MWImageIOFactory; }

  /** Run-time type information (and related methods). */
  itkTypeMacro(MWImageIOFactory, itk::ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    MWImageIOFactory::Pointer MWFactory = MWImageIOFactory::New();
    itk::ObjectFactoryBase::RegisterFactory(MWFactory);
  }

protected:
  MWImageIOFactory();
  ~MWImageIOFactory() ITK_OVERRIDE;

private:
  MWImageIOFactory(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

};

} // end namespace otb

#endif
