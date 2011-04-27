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
#include "otbWrapperApplication.h"

namespace otb
{
namespace Wrapper
{

Application::Application(): m_Name(""), m_Description("")
{
}

Application::~Application()
{
}

void Application::Init()
{
  m_ParameterList = ParameterGroup::New();
  this->DoCreateParameters();
}

void Application::UpdateParameters()
{
  this->DoUpdateParameters();
}

void Application::Execute()
{
  this->DoExecute();
}

}
}

