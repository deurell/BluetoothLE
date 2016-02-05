//
// App.xaml.cpp
// Implementation of the App class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <sstream>
#include <iomanip>

using namespace BluetoothLE;
using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace Windows::Storage::Streams;

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
	InitializeComponent();
	Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);

	//SetupAdvertiser();
	SetupWatcher();
}

/// <summary>
/// Invoked when the application is launched normally by the end user.	Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{

#if _DEBUG
		// Show graphics profiling information while debugging.
		if (IsDebuggerPresent())
		{
			// Display the current frame rate counters
			 DebugSettings->EnableFrameRateCounter = true;
		}
#endif

	auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

	// Do not repeat app initialization when the Window already has content,
	// just ensure that the window is active
	if (rootFrame == nullptr)
	{
		// Create a Frame to act as the navigation context and associate it with
		// a SuspensionManager key
		rootFrame = ref new Frame();

		rootFrame->NavigationFailed += ref new Windows::UI::Xaml::Navigation::NavigationFailedEventHandler(this, &App::OnNavigationFailed);

		if (e->PreviousExecutionState == ApplicationExecutionState::Terminated)
		{
			// TODO: Restore the saved session state only when appropriate, scheduling the
			// final launch steps after the restore is complete

		}

		if (rootFrame->Content == nullptr)
		{
			// When the navigation stack isn't restored navigate to the first page,
			// configuring the new page by passing required information as a navigation
			// parameter
			rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
		}
		// Place the frame in the current Window
		Window::Current->Content = rootFrame;
		// Ensure the current window is active
		Window::Current->Activate();
	}
	else
	{
		if (rootFrame->Content == nullptr)
		{
			// When the navigation stack isn't restored navigate to the first page,
			// configuring the new page by passing required information as a navigation
			// parameter
			rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
		}
		// Ensure the current window is active
		Window::Current->Activate();
	}
}

/// <summary>
/// Invoked when application execution is being suspended.	Application state is saved
/// without knowing whether the application will be terminated or resumed with the contents
/// of memory still intact.
/// </summary>
/// <param name="sender">The source of the suspend request.</param>
/// <param name="e">Details about the suspend request.</param>
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
	(void) sender;	// Unused parameter
	(void) e;	// Unused parameter

	//TODO: Save application state and stop any background activity
}

/// <summary>
/// Invoked when Navigation to a certain page fails
/// </summary>
/// <param name="sender">The Frame which failed navigation</param>
/// <param name="e">Details about the navigation failure</param>
void App::OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e)
{
	throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}

void BluetoothLE::App::SetupAdvertiser() {
	mPublisher = ref new BluetoothLEAdvertisementPublisher();
	mPublisher->StatusChanged += ref new TypedEventHandler<BluetoothLEAdvertisementPublisher ^, BluetoothLEAdvertisementPublisherStatusChangedEventArgs ^>(this, &BluetoothLE::App::OnStatusChanged);
	auto manifacturerData = ref new BluetoothLEManufacturerData();
	manifacturerData->CompanyId = 0xFFFE;
	auto dataWriter = ref new DataWriter();
	unsigned short iiidData = 0x1234;
	dataWriter->WriteUInt16(iiidData);
	manifacturerData->Data = dataWriter->DetachBuffer();
	mPublisher->Advertisement->ManufacturerData->Append(manifacturerData);
	mPublisher->Start();
}

void BluetoothLE::App::OnStatusChanged(BluetoothLEAdvertisementPublisher ^sender, BluetoothLEAdvertisementPublisherStatusChangedEventArgs ^args) {
	wchar_t buffer[256];
	swprintf_s(buffer, 256, L"Bluetooth advertisement status: %s\n", args->Status.ToString()->Data());
	OutputDebugStringW((wchar_t*)buffer);
}

void BluetoothLE::App::SetupWatcher() {
	mWatcher = ref new BluetoothLEAdvertisementWatcher();
	auto manufacturerData = ref new BluetoothLEManufacturerData();
	manufacturerData->CompanyId = 0xFFFE;
	auto writer = ref new DataWriter();
	writer->WriteUInt16(0x1234);
	manufacturerData->Data = writer->DetachBuffer();
	mWatcher->AdvertisementFilter->Advertisement->ManufacturerData->Append(manufacturerData);
	mWatcher->SignalStrengthFilter->InRangeThresholdInDBm = ref new Platform::Box<short>(-70);
	mWatcher->SignalStrengthFilter->OutOfRangeThresholdInDBm = ref new Platform::Box<short>(-75);
	auto timeSpan = Windows::Foundation::TimeSpan();
	timeSpan.Duration = 20000 * 1000;
	mWatcher->SignalStrengthFilter->OutOfRangeTimeout = timeSpan;

	mWatcher->Received += ref new TypedEventHandler<BluetoothLEAdvertisementWatcher ^, BluetoothLEAdvertisementReceivedEventArgs ^>(this, &BluetoothLE::App::OnAdvertisementReceived);
	mWatcher->Start();
}


void BluetoothLE::App::OnAdvertisementReceived(BluetoothLEAdvertisementWatcher ^sender, BluetoothLEAdvertisementReceivedEventArgs ^args) {
	auto timestamp = args->Timestamp;
	auto advertisementType = args->AdvertisementType;
	auto rssi = args->RawSignalStrengthInDBm;
	auto localName = args->Advertisement->LocalName;

	auto manifacturerSections = args->Advertisement->ManufacturerData;
	if (manifacturerSections->Size > 0) {
		auto manufacturerData = manifacturerSections->First();
		Platform::Array<byte>^ data = ref new Platform::Array<byte>(manufacturerData->Current->Data->Length);
		auto reader = DataReader::FromBuffer(manufacturerData->Current->Data);
		reader->ReadBytes(data);
		
		// just send the bytes as a string to debug output for now
		std::stringstream stringStream;
		stringStream << std::hex << std::setfill('0') << std::uppercase;
		for (int i = 0; i < data->Length; ++i) {
			stringStream << (int)data[i];
		}
		OutputDebugStringA(stringStream.str().c_str());
	}
}
