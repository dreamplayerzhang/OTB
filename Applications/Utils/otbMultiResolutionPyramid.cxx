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
#include "otbWrapperApplicationFactory.h"

#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbPerBandVectorImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkShrinkImageFilter.h"
#include "otbImageFileReader.h"
#include "otbObjectList.h"
#include "otbStreamingImageFileWriter.h"
#include "otbCommandLineArgumentParser.h"
#include "otbStandardWriterWatcher.h"

#include "otbWrapperParameter.h"
#include "otbWrapperOutputImageParameter.h"

namespace otb
{
namespace Wrapper
{

class MultiResolutionPyramid : public Application
{

public:
  /** Standard class typedefs. */
  typedef MultiResolutionPyramid        Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(MultiResolutionPyramid, otb::Application);
  
  /** Image and filters typedef */
  typedef otb::Image<float>                                          SingleImageType;
  typedef itk::DiscreteGaussianImageFilter<SingleImageType,  
                                           SingleImageType>          SmoothingImageFilterType;

  typedef otb::PerBandVectorImageFilter<FloatVectorImageType, 
                                        FloatVectorImageType,
                                        SmoothingImageFilterType>     SmoothingVectorImageFilterType;

  typedef itk::ShrinkImageFilter<FloatVectorImageType, 
                                 FloatVectorImageType>                ShrinkFilterType;

private:
  MultiResolutionPyramid()
  {
    SetName("MultiResolutionPyramid");
    SetDescription("Build a multi-resolution pyramid of the image.");
  }

  void DoCreateParameters()
  {
//     descriptor->SetName("Multi-resolution pyramid tool");
//   descriptor->SetDescription("Build a multi-resolution pyramid of the image.");
//   descriptor->AddInputImage();
//   descriptor->AddOption("NumberOfLevels","Number of levels in the pyramid (default is 1)","level", 1, false, ApplicationDescriptor::Integer);
//   descriptor->AddOption("ShrinkFactor","Subsampling factor (default is 2)","sfactor", 1, false, ApplicationDescriptor::Integer);
//   descriptor->AddOption("VarianceFactor","Before subsampling, image
//   is smoothed with a gaussian kernel of variance
//   VarianceFactor*ShrinkFactor. 
//Higher values will result in more blur, lower in more aliasing (default is 0.6)","vfactor", 1, false, ApplicationDescriptor::Real);
//   descriptor->AddOption("FastScheme","If used, this option allows to speed-up computation by iteratively subsampling previous level of pyramid instead of processing the full input image each time. Please note that this may result in extra blur or extra aliasing.","fast", 0, false, ApplicationDescriptor::Integer);
//   descriptor->AddOption("OutputPrefixAndExtextension","prefix for the output files, and extension","out", 2, true, ApplicationDescriptor::String);
//   descriptor->AddOption("AvailableMemory","Set the maximum of available memory for the pipeline execution in mega bytes (optional, 256 by default)","ram", 1, false, otb::ApplicationDescriptor::Integer);


    AddParameter(ParameterType_InputImage,  "in",   "Input Image");

    AddParameter(ParameterType_OutputImage, "out",  "Output Image");
    SetParameterDescription( "out","will be used to get the prefix and the extension of the images to write");

    AddParameter(ParameterType_Int, "level", "Number Of Levels");
    SetParameterInt("level", 1);
    SetParameterDescription( "level", "Number of levels in the pyramid (default is 1)");
    
    AddParameter(ParameterType_Int, "sfactor", "Subsampling factor");
    SetParameterInt("sfactor", 2);

    AddParameter(ParameterType_Float,  "vfactor", "Subsampling factor");
    SetParameterFloat("vfactor", 0.6);

    // Boolean Fast scheme
        
    // Available memory
    
    
//     MandatoryOff("outmin");
//     MandatoryOff("outmax");
  }


  void DoUpdateParameters()
  {
    // Nothing to do here for the parameters : all are independent
    
    // Reinitialize the internal process used
  }

  void DoExecute()
  {
    // Initializing the process
    m_SmoothingFilter =  SmoothingVectorImageFilterType::New();
    m_ShrinkFilter    = ShrinkFilterType::New();

    // Extract Parameters
    unsigned int nbLevels     = GetParameterInt("level");
    unsigned int shrinkFactor = GetParameterInt("sfactor");
    double varianceFactor     = GetParameterFloat("vfactor");
    
    //bool fastScheme = parseResult->IsOptionPresent("FastScheme");
    bool fastScheme = false;

    // Get the input image
    FloatVectorImageType::Pointer inImage = GetParameterImage("in");


    // Get the Initial Output Image FileName
    Parameter* param = GetParameterByKey("out");
    std::string path, fname, ext;
    if (dynamic_cast<OutputImageParameter*>(param))
      {
      OutputImageParameter* paramDown = dynamic_cast<OutputImageParameter*>(param);
      std::string ofname = paramDown->GetFileName();

      // Get the extension and the prefix of the filename
      path  = itksys::SystemTools::GetFilenamePath(ofname);
      fname = itksys::SystemTools::GetFilenameWithoutExtension(ofname);
      ext   = itksys::SystemTools::GetFilenameExtension(ofname);
      }

    unsigned int currentLevel = 1;
    unsigned int currentFactor = shrinkFactor;

    while(currentLevel <= nbLevels)
      {
      otbAppLogDEBUG( << "Processing level " << currentLevel << " with shrink factor "<<currentFactor);
      
      m_SmoothingFilter->SetInput(inImage);

      // According to
      // http://www.ipol.im/pub/algo/gjmr_line_segment_detector/
      // This is a good balance between blur and aliasing
      double variance = varianceFactor * static_cast<double>(currentFactor);
      m_SmoothingFilter->GetFilter()->SetVariance(variance);

      m_ShrinkFilter->SetInput(m_SmoothingFilter->GetOutput());
      m_ShrinkFilter->SetShrinkFactors(currentFactor);


      if(!fastScheme)
        {
        currentFactor*=shrinkFactor;
        }
           
      // Get the Output Parameter to change the current image filename
      Parameter* param = GetParameterByKey("out");      
      if (dynamic_cast<OutputImageParameter*>(param))
        {
        OutputImageParameter* paramDown = dynamic_cast<OutputImageParameter*>(param);
        
        // build the current image filename
        std::ostringstream oss;
        oss <<path<<"/"<<fname<<"_"<<currentLevel<<ext;
        
        // writer label
        std::ostringstream osswriter;
        osswriter<< "writer "<< currentLevel;

        // Set the filename of the current output image
        paramDown->SetFileName(oss.str());

        // Add the current level to be written
        SetParameterOutputImage("out", m_ShrinkFilter->GetOutput());
        paramDown->InitializeWriters();
        AddProcess(paramDown->GetWriter(),osswriter.str());
        paramDown->Write();
        }
      ++currentLevel;
      }
    
    // Disable the output Image parameter to avoid writing 
    // the last image (Application::ExecuteAndWriteOutput method)
    GetParameterByKey("out")->SetActive(false);
  }

  SmoothingVectorImageFilterType::Pointer   m_SmoothingFilter;
  ShrinkFilterType::Pointer                 m_ShrinkFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::MultiResolutionPyramid)
