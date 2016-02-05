//
// App.xaml.h
// Declaration of the App class.
//

#pragma once

#include "App.g.h"

using namespace Windows::Devices::Bluetooth::Advertisement;

namespace BluetoothLE
{
	/// <summary>
	/// Provides application-specific behavior to supplement the default Application class.
	/// </summary>
	ref class App sealed
	{
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	internal:
		App();

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);

		void SetupAdvertiser();
		void OnStatusChanged(Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementPublisher ^sender, Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementPublisherStatusChangedEventArgs ^args);

		void SetupWatcher();

		BluetoothLEAdvertisementPublisher^ mPublisher;
		BluetoothLEAdvertisementWatcher^ mWatcher;
		void OnAdvertisementReceived(Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher ^sender, Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs ^args);
	};
}