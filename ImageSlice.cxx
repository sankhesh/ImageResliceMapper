#include <vtkVersion.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkImageMapper.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkRendererCollection.h>
#include <vtkAbstractPicker.h>
#include <vtkObjectFactory.h>
#include <vtkXMLImageDataReader.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkImageProperty.h>
#include <vtkPlane.h>

static int step = 0;
static void CreateColorImage(vtkImageData*);
class MouseInteractorStylePP : public vtkInteractorStyleTrackballCamera
{
public:
  static MouseInteractorStylePP* New();
  vtkTypeMacro(MouseInteractorStylePP, vtkInteractorStyleTrackballCamera);

  vtkSmartPointer<vtkImageResliceMapper> mapper;
  double origin, spacing;
  double ycenter, zcenter;

  virtual void OnRightButtonDown()
    {
    std::cout << "Step slice through: " << step++ << std::endl;

    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(origin + step*spacing, ycenter, zcenter);
    plane->SetNormal(1.0, 0.0, 0.0);
    this->mapper->SetSlicePlane(plane);
    vtkInteractorStyleTrackballCamera::OnRightButtonDown();
    }
//  virtual void OnLeftButtonDown()
//    {
//    std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
//
//    this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
//                                        this->Interactor->GetEventPosition()[1],
//                                        0,  // always zero.
//                                        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
//    double picked[3];
//    this->Interactor->GetPicker()->GetPickPosition(picked);
//    std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;
//    // Forward events
//    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
//    }
};

vtkStandardNewMacro(MouseInteractorStylePP);

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
    std::cerr << "Usage: " << argv[0] << " <input filename (*.vti)>" << std::endl;
    return EXIT_FAILURE;
    }
  vtkSmartPointer<vtkXMLImageDataReader> reader =
    vtkSmartPointer<vtkXMLImageDataReader>::New();
  reader->SetFileName(argv[1]);
  reader->Update();
  vtkSmartPointer<vtkImageData> im = reader->GetOutput();

  vtkSmartPointer<vtkImageResliceMapper> imageResliceMapper = vtkSmartPointer<vtkImageResliceMapper>::New();
  imageResliceMapper->SetInputConnection(reader->GetOutputPort());

  vtkSmartPointer<vtkImageSlice> imageSlice = vtkSmartPointer<vtkImageSlice>::New();
  imageSlice->SetMapper(imageResliceMapper);

  // Setup volume
  vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper =
    vtkSmartPointer<vtkSmartVolumeMapper>::New();
  volumeMapper->SetInputConnection(reader->GetOutputPort());

  vtkSmartPointer<vtkColorTransferFunction> ctf =
    vtkSmartPointer<vtkColorTransferFunction>::New();
  ctf->AddRGBPoint(0, 0.23, 0.3, 0.75);
  ctf->AddRGBPoint(127.5, 0.86, 0.86, 0.86);
  ctf->AddRGBPoint(255, 0.7, 0.015, 0.14);

  vtkSmartPointer<vtkPiecewiseFunction> pf =
    vtkSmartPointer<vtkPiecewiseFunction>::New();
  pf->AddPoint(0, 0);
  pf->AddPoint(255, 0.1);

  vtkSmartPointer<vtkVolumeProperty> vp =
    vtkSmartPointer<vtkVolumeProperty>::New();
  vp->SetColor(ctf);
  vp->SetScalarOpacity(pf);

  vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
  volume->SetMapper(volumeMapper);
  volume->SetProperty(vp);

  // Pass ctf to slice
  vtkSmartPointer<vtkImageProperty> imp =
    vtkSmartPointer<vtkImageProperty>::New();
  imp->SetLookupTable(ctf);
  imp->SetInterpolationTypeToLinear();
  imageSlice->SetProperty(imp);

  // Setup renderers
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->AddViewProp(imageSlice);
  renderer->AddVolume(volume);
  renderer->ResetCamera();

  // Setup render window
  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->SetSize(300, 300);
  renderWindow->AddRenderer(renderer);

  // Setup render window interactor
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();

  vtkSmartPointer<MouseInteractorStylePP> style =
    vtkSmartPointer<MouseInteractorStylePP>::New();

  double bounds[6];
  im->GetBounds(bounds);
  style->mapper = imageResliceMapper;
  style->origin = im->GetOrigin()[0];
  style->spacing = im->GetSpacing()[0];
  style->ycenter = (bounds[2] + bounds[3])/2.0;
  style->zcenter = (bounds[4] + bounds[5])/2.0;
  renderWindowInteractor->SetInteractorStyle(style);

  // Render and start interaction
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindowInteractor->Initialize();

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

void CreateColorImage(vtkImageData* image)
{
  image->SetDimensions(10, 10, 10);
#if VTK_MAJOR_VERSION <= 5
  image->SetScalarTypeToUnsignedChar();
  image->SetNumberOfScalarComponents(3);
  image->AllocateScalars();
#else
  image->AllocateScalars(VTK_UNSIGNED_CHAR,3);
#endif
  for(unsigned int x = 0; x < 10; x++)
    {
    for(unsigned int y = 0; y < 10; y++)
      {
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(x,y,0));
      pixel[0] = 255;
      pixel[1] = 0;
      pixel[2] = 255;
      }
    }
}
