/*
 * Copyright (C) 2005-2017 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"

#include "otbContrastEnhancementFilter.h"

namespace otb
{
namespace Wrapper
{

class ContrastEnhancement : public Application
{
public:
  /** Standard class typedefs. */
  typedef ContrastEnhancement	              Self;
  typedef Application	                      Superclass;
  typedef itk::SmartPointer < Self >	      Pointer;
  typedef itk::SmartPointer < const Self >	ConstPointer;

  typedef otb::ContrastEnhancementFilter < FloatImageType , FloatImageType >
  				FilterType;
  typedef otb :: ImageList< FloatImageType > ImageListType;
  typedef otb::VectorImageToImageListFilter< FloatVectorImageType, ImageListType > 
          VectorToImageListFilterType;
  typedef otb::ImageListToVectorImageFilter< ImageListType, FloatVectorImageType > 
          ImageListToVectorFilterType;

  /** Standard macro */
  itkNewMacro( Self );
 
  itkTypeMacro( ContrastEnhancement , otb::Application );

private:
	void DoInit() ITK_OVERRIDE
	{
		SetName("Contrast Enhancement");
    SetDescription("");

    // Documentation
    SetDocName("Contrast Enhancement");
    SetDocLongDescription("");
    SetDocLimitations("None");
    SetDocAuthors("OTB-Team");
    SetDocSeeAlso("");

    AddDocTag(Tags::Filter);

    AddParameter(ParameterType_InputImage,  "in",   "Input Image");
    SetParameterDescription("in", "Input image.");
    SetParameterString("in", "/home/antoine/dev/my_data/lena/lena.jpg");
    AddParameter(ParameterType_OutputImage, "out",  "Output Image");
    SetParameterDescription("out", "Output image.");
    SetParameterString("out", "/home/antoine/dev/my_data/lenaapp.jpg");


    AddParameter(ParameterType_Int,"bin","Bin number");
    SetDefaultParameterInt("bin", 256);
    SetParameterDescription("bin","Number of bin in the histogram");

    AddParameter(ParameterType_Float,"hfact","contrast Limitation");  
    SetParameterDescription("hfact","This parameter will set the maximum "
      "height accepted in a bin on the histogram of the input image. "
      "the height will be computated as hfact*eqHeight where eqHeight "
      "is the height of the theorical flat histogram");
    MandatoryOff("hfact");

    AddParameter(ParameterType_Float,"nodata","nodata value");
    MandatoryOff("nodata");

    AddParameter(ParameterType_Group, "thumb", "Thumbnail size");

    AddParameter(ParameterType_Int,"thumb.h","Thumbnail height in pixel");
    AddParameter(ParameterType_Int,"thumb.w","Thumbnail width in pixel");

    AddParameter(ParameterType_Choice , "mode" , "What to equalized");
    AddChoice( "mode.each" , "Channels" );
    SetParameterDescription( "mode.each" ,
                "Each channel are equalized independently" );
    AddChoice( "mode.lum" , "Luminance" );
    SetParameterDescription( "mode.lum" ,
                "The luminance is equalized and then a gain is applied "
                "on the channels." );
    AddParameter(ParameterType_Group , "mode.lum.red" , "Red Channel" );
    AddParameter(ParameterType_Int , "mode.lum.red.ch" , "Red Channel" );
    SetDefaultParameterInt("mode.lum.red.ch", 0 );
    AddParameter(ParameterType_Float , "mode.lum.red.coef" ,
                 "Value for luminance computation" );
    SetDefaultParameterFloat("mode.lum.red.coef", 0.21 );

    AddParameter(ParameterType_Group , "mode.lum.gre" , "Green Channel" );
    AddParameter(ParameterType_Int , "mode.lum.gre.ch" , "Green Channel" );
    SetDefaultParameterInt("mode.lum.gre.ch", 1 );
    AddParameter(ParameterType_Float , "mode.lum.gre.coef" ,
                 "Value for luminance computation" );
    SetDefaultParameterFloat("mode.lum.gre.coef", 0.71 );

    AddParameter(ParameterType_Group , "mode.lum.blu" , "Blue Channel" );
    AddParameter(ParameterType_Int , "mode.lum.blu.ch" , "Blue Channel" );
    SetDefaultParameterInt("mode.lum.blu.ch", 2 );
    AddParameter(ParameterType_Float , "mode.lum.blu.coef" ,
                 "Value for luminance computation" );
    SetDefaultParameterFloat("mode.lum.blu.coef", 0.08 );

    SetMinimumParameterIntValue("mode.lum.red.ch", 0);
    SetMinimumParameterIntValue("mode.lum.gre.ch", 0);
    SetMinimumParameterIntValue("mode.lum.blu.ch", 0);
    SetMinimumParameterIntValue("bin", 2);
    SetMinimumParameterIntValue("thumb.h", 0);
    SetMinimumParameterIntValue("thumb.w", 0);

    AddRAMParameter();

    
  }

  void DoUpdateParameters() ITK_OVERRIDE
  {
    if (HasValue("in") )
      {
      FloatVectorImageType * inImage = GetParameterImage("in");
      if ( !HasUserValue("thumb.w") )
        {
        SetParameterInt( "thumb.w" , 
          inImage->GetLargestPossibleRegion().GetSize()[0] );
        }
      if ( !HasUserValue("thumb.h") )
        {
      SetParameterInt( "thumb.h" , 
        inImage->GetLargestPossibleRegion().GetSize()[1] );
        }
      {
      std::ostringstream oss;
      if ( HasUserValue("thumb.h") && 
        GetParameterImage("in")->GetLargestPossibleRegion().GetSize()[1]%GetParameterInt("thumb.h") != 0 )
        {
        oss <<"error : hThumbnail = "<<GetParameterInt("thumb.h")<<
              " is not a divider of the input's height"<<std::endl;
        oss<<"Image Height = "<<GetParameterImage("in")->GetLargestPossibleRegion().GetSize()[1]<<std::endl;
        // std::cout<<"error : hThumbnail = "<<GetParameterInt("thumb.h")<<" is not a divider of the input's height"<<std::endl;
        // std::cout<<"Image Height = "<<GetParameterImage("in")->GetLargestPossibleRegion().GetSize()[1]<<std::endl;
        }
      if ( HasUserValue("thumb.w") && 
        GetParameterImage("in")->GetLargestPossibleRegion().GetSize()[0]%GetParameterInt("thumb.w") != 0 )
        {   
        oss <<"error : wThumbnail = "<<GetParameterInt("thumb.w")<<
              " is not a divider of the input's width"<<std::endl;
        oss<<"Image Width = "<<GetParameterImage("in")->GetLargestPossibleRegion().GetSize()[1]<<std::endl;
        // std::cout<<"error : wThumbnail = "<<GetParameterInt("thumb.w")<<"is not a divider of the input's width"<<std::endl;
        // std::cout<<"Image Width = "<<GetParameterImage("in")->GetLargestPossibleRegion().GetSize()[0]<<std::endl;
        }
      otbAppLogINFO( << oss.str() );
      }
      if ( !HasUserValue("nodata") )
        {
        typedef ImageMetadataInterfaceBase ImageMetadataInterfaceType;
        ImageMetadataInterfaceType::Pointer metadataInterface = 
            ImageMetadataInterfaceFactory::CreateIMI(inImage->GetMetaDataDictionary());
        std::vector<double> values;
        std::vector<bool> flags;

        bool ret = metadataInterface->GetNoDataFlags(flags,values);

        if(ret && !values.empty() && !flags.empty() && flags[0])
          {
          SetParameterFloat( "nodata" , static_cast<float>( values[0] ) );
          }
        if ( GetParameterString( "mode" ) == "lum" )
          {
          std::vector<uint> rgb = metadataInterface->GetDefaultDisplay() ;
          uint m = inImage->GetVectorLength ();
          SetParameterInt( "mode.lum.red.ch" , rgb[0] );
          SetParameterInt( "mode.lum.gre.ch" , rgb[1] );
          SetParameterInt( "mode.lum.blu.ch" , rgb[2] );
          if( m < rgb[ 0 ] )
            {
            SetParameterFloat ("mode.lum.red.coef" , 0.0 );
            SetParameterInt( "mode.lum.red.ch" , 0 );
            }
          if( m < rgb[ 1 ] )
            {
            SetParameterFloat ("mode.lum.gre.coef" , 0.0 );
            SetParameterInt( "mode.lum.gre.ch" , 0 );
            }
          if( m < rgb[ 2 ] )
            {
            SetParameterFloat ("mode.lum.blu.coef" , 0.0 );
            SetParameterInt( "mode.lum.blu.ch" , 0 );
            }
          }
        }
      }
  }

  void DoExecute() ITK_OVERRIDE
  {
    ImageListToVectorFilterType::Pointer 
          imageListToVectorFilterOut( ImageListToVectorFilterType::New() );
    FloatVectorImageType * inImage = GetParameterImage("in");
    ImageListType::Pointer outputImageList ( ImageListType::New() );
    VectorToImageListFilterType::Pointer 
          vectorToImageListFilter ( VectorToImageListFilterType::New() );
    vectorToImageListFilter->SetInput( inImage );
    vectorToImageListFilter->Update();
    ImageListType::Pointer inputImageList = vectorToImageListFilter->GetOutput();

    if ( GetParameterString("mode") == "each")
      {
      // Each channel will be equalized
      int m = inImage->GetVectorLength ();
      for (int chanel = 0 ; chanel<m ; chanel++ ) 
        {
        // std::cout<<"channel m ="<<m<<std::endl;
        FilterType::Pointer filter( FilterType::New() );
        filter->SetInput( inputImageList->GetNthElement(chanel) ) ;
        if ( HasValue("hfact") )
          {
          filter->SetHistoThreshFactor( GetParameterInt("hfact") );
          }
        filter->SetHistoSize(GetParameterInt("bin"));
        if ( HasUserValue("nodata") )
          {
          filter->SetNoData( GetParameterFloat("nodata") );
          }
        filter->SetThumbnailSize( GetParameterInt("thumb.w") , 
                                GetParameterInt("thumb.h") );
        filter->Update();
        outputImageList->PushBack( filter->GetOutput() );
        }
      }

    if ( GetParameterString("mode") == "lum")
      {
      // Retreive order of the RGB channels
      std::vector<int> rgb(3 , 0);
      rgb[0] = GetParameterInt("mode.lum.red.ch");
      rgb[1] = GetParameterInt("mode.lum.gre.ch");
      rgb[2] = GetParameterInt("mode.lum.blu.ch");
      // Retreive the coeff for each channel
      std::vector<float> lumCoef(3 , 0.0);
      lumCoef[0] = GetParameterFloat("mode.lum.red.coef");
      lumCoef[1] = GetParameterFloat("mode.lum.gre.coef");
      lumCoef[2] = GetParameterFloat("mode.lum.blu.coef");
      // Normalize those coeff
      float sum = 0.0;
      for (float f : lumCoef)
        {
        sum +=f;
        }
      assert(sum>0);
      for (int i = 0 ; i<3 ; i++ )
        {
        lumCoef[1] /= sum;
        }
      itk::ImageRegionIterator < FloatImageType > 
          rit ( inputImageList->GetNthElement(rgb[0]) ,
               inputImageList->GetNthElement(rgb[0])->GetRequestedRegion() );
      itk::ImageRegionIterator < FloatImageType > 
          git ( inputImageList->GetNthElement(rgb[1]) ,
               inputImageList->GetNthElement(rgb[1])->GetRequestedRegion() );
      itk::ImageRegionIterator < FloatImageType > 
          bit ( inputImageList->GetNthElement(rgb[2]) ,
               inputImageList->GetNthElement(rgb[2])->GetRequestedRegion() );
      // Create Luminance image 
      FloatImageType::Pointer luminance(FloatImageType::New());
      luminance->Allocate();
      luminance->SetRegions( inputImageList->GetNthElement(rgb[0])->GetLargestPossibleRegion() );
      luminance->SetOrigin( inputImageList->GetNthElement(rgb[0])->GetOrigin() );
      luminance->SetSpacing( inputImageList->GetNthElement(rgb[0])->GetSpacing() );

      itk::ImageRegionIterator < FloatImageType > 
          lit ( luminance ,
               luminance->GetRequestedRegion() );
      lit.GoToBegin();
      bit.GoToBegin();
      rit.GoToBegin();
      git.GoToBegin();
      while ( !lit.IsAtEnd())
        {
        lit.Set( lumCoef[0] * rit.Get() + lumCoef[1] * git.Get() + lumCoef[2] * bit.Get() );
        ++lit;
        ++bit;
        ++rit;
        ++git;
        }

      // Apply equalization on the luminance
      FilterType::Pointer filter( FilterType::New() );
      filter->SetInput( luminance ) ;
      if ( HasValue("hfact") )
        {
        filter->SetHistoThreshFactor( GetParameterInt("hfact") );
        }
      filter->SetHistoSize(GetParameterInt("bin"));
      // Nodata has to be the same for all the channels
      if ( HasUserValue("nodata") )
        {
        filter->SetNoData( GetParameterFloat("nodata") );
        }
      filter->SetThumbnailSize( GetParameterInt("thumb.w") , 
                                GetParameterInt("thumb.h") );
      filter->Update();

      itk::ImageRegionIterator < FloatImageType > 
          nlit ( filter->GetOutput() , 
                filter->GetOutput()->GetRequestedRegion() );
      lit.GoToBegin();
      nlit.GoToBegin();
      bit.GoToBegin();
      rit.GoToBegin();
      git.GoToBegin();
      float gain = 0.0;
      float denum = 1;
      // Compute the gain F(luminance)/luminance
      // Check if the creation of a gain image and a multiplication 
      // would be more efficient (memory and time)
      while ( !lit.IsAtEnd())
        {
        if ( lit.Get() != 0 )
          denum = lit.Get();
        else
          {
          denum = 1;
          }
        gain = nlit.Get()/denum;
        bit.Set(gain * bit.Get());
        rit.Set(gain * rit.Get());
        git.Set(gain * git.Get());
        ++nlit;
        ++lit;
        ++bit;
        ++rit;
        ++git;
        }
      outputImageList = inputImageList;
      }
    imageListToVectorFilterOut->SetInput(outputImageList);
    imageListToVectorFilterOut->Update();
    SetParameterOutputImage( "out" , imageListToVectorFilterOut->GetOutput() );
  }
};

} //End namespace Wrapper
} //End namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::ContrastEnhancement)