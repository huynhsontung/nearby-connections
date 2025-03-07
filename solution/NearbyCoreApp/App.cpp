#include "pch.h"
#include <winrt/NearbyLibrary.h>
#include <sstream>

using namespace winrt;

using namespace Windows;
using namespace Windows::Foundation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    CompositionTarget m_target{ nullptr };
    VisualCollection m_visuals{ nullptr };
    Visual m_selected{ nullptr };
    float2 m_offset{};
    NearbyLibrary::Nearby m_nearby{};
    NearbyLibrary::Nearby::EndpointFound_revoker m_endpoint_found_revoker{};
    NearbyLibrary::Nearby::EndpointDistanceChanged_revoker m_endpoint_distance_changed_revoker{};

    IAsyncAction StartAdvertising()
    {
        co_await winrt::resume_background();
        OutputDebugString(L"Advertising\n");
        auto options = NearbyLibrary::ConnectionOptions{};
        options.Strategy(NearbyLibrary::Strategy::P2pCluster);
        const auto mediumSelector = NearbyLibrary::MediumSelector{};
        mediumSelector.WifiLan(true);
        //mediumSelector.Bluetooth(true);
        options.Allowed(mediumSelector);
        options.KeepAliveIntervalMillis(5000);
        options.KeepAliveTimeoutMillis(30000);
        auto value = m_nearby.StartAdvertisingAsync(L"NearbySharing", options, L"com.google.android.gms.auth.proximity.START_NEARBY").get();
        OutputDebugString(L"Here!\n");
    }

    IAsyncAction StartDiscovering()
    {
        co_await winrt::resume_background();
        OutputDebugString(L"Discovering\n");
        auto options = NearbyLibrary::ConnectionOptions{};
        options.Strategy(NearbyLibrary::Strategy::P2pCluster);
        const auto mediumSelector = NearbyLibrary::MediumSelector{};
        mediumSelector.WifiLan(true);
        mediumSelector.Bluetooth(true);
        options.Allowed(mediumSelector);
        options.KeepAliveIntervalMillis(5000);
        options.KeepAliveTimeoutMillis(30000);
        auto value = m_nearby.StartDiscoveryAsync(L"NearbySharing", options).get();
        OutputDebugString(L"Here!\n");
    }

    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const &)
    {
        m_endpoint_found_revoker = m_nearby.EndpointFound(winrt::auto_revoke,
        [](NearbyLibrary::Nearby core, NearbyLibrary::EndpointFoundEventArgs args)
        {
            std::wostringstream debugMessage;
            debugMessage << L"Endpoint Found:\n\tservice_id=" << args.ServiceId().c_str()
              << L"\n\tendpoint_id=" << args.EndpointId().c_str()
              << L"\n\tendpoint_info=" << args.EndpointInfo().c_str() << std::endl;
            OutputDebugString(debugMessage.str().c_str());
        });
        m_endpoint_distance_changed_revoker = m_nearby.EndpointDistanceChanged(winrt::auto_revoke,
          [](NearbyLibrary::Nearby core, NearbyLibrary::EndpointDistanceChangedEventArgs args)
          {
            std::wostringstream debugMessage;
            debugMessage << L"Endpoint Distance Changed:\n\endpoint_id=" << args.EndpointId().c_str() << std::endl;
            OutputDebugString(debugMessage.str().c_str());
          });
    }

    void Load(hstring const&)
    {
    }

    void Uninitialize()
    {
    }

    void Run()
    {
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();

        CoreDispatcher dispatcher = window.Dispatcher();
        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    }

    void SetWindow(CoreWindow const & window)
    {
        Compositor compositor;
        ContainerVisual root = compositor.CreateContainerVisual();
        m_target = compositor.CreateTargetForCurrentView();
        m_target.Root(root);
        m_visuals = root.Children();

        window.PointerPressed({ this, &App::OnPointerPressed });
        window.PointerMoved({ this, &App::OnPointerMoved });

        window.PointerReleased([&](auto && ...)
        {
            m_selected = nullptr;
        });
    }

    void OnPointerPressed(IInspectable const &, PointerEventArgs const & args)
    {
        float2 const point = args.CurrentPoint().Position();

        for (Visual visual : m_visuals)
        {
            float3 const offset = visual.Offset();
            float2 const size = visual.Size();

            if (point.x >= offset.x &&
                point.x < offset.x + size.x &&
                point.y >= offset.y &&
                point.y < offset.y + size.y)
            {
                m_selected = visual;
                m_offset.x = offset.x - point.x;
                m_offset.y = offset.y - point.y;
            }
        }

        if (m_selected)
        {
            m_visuals.Remove(m_selected);
            m_visuals.InsertAtTop(m_selected);
        }
        else
        {
            AddVisual(point);
        }

        StartDiscovering();
    }

    void OnPointerMoved(IInspectable const &, PointerEventArgs const & args)
    {
        if (m_selected)
        {
            float2 const point = args.CurrentPoint().Position();

            m_selected.Offset(
            {
                point.x + m_offset.x,
                point.y + m_offset.y,
                0.0f
            });
        }
    }

    void AddVisual(float2 const point)
    {
        Compositor compositor = m_visuals.Compositor();
        SpriteVisual visual = compositor.CreateSpriteVisual();

        static Color colors[] =
        {
            { 0xDC, 0x5B, 0x9B, 0xD5 },
            { 0xDC, 0xED, 0x7D, 0x31 },
            { 0xDC, 0x70, 0xAD, 0x47 },
            { 0xDC, 0xFF, 0xC0, 0x00 }
        };

        static unsigned last = 0;
        unsigned const next = ++last % _countof(colors);
        visual.Brush(compositor.CreateColorBrush(colors[next]));

        float const BlockSize = 100.0f;

        visual.Size(
        {
            BlockSize,
            BlockSize
        });

        visual.Offset(
        {
            point.x - BlockSize / 2.0f,
            point.y - BlockSize / 2.0f,
            0.0f,
        });

        m_visuals.InsertAtTop(visual);

        m_selected = visual;
        m_offset.x = -BlockSize / 2.0f;
        m_offset.y = -BlockSize / 2.0f;
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
