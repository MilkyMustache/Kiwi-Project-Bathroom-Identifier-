import CoreBluetooth
import Foundation

// Matches the service data broadcast by bathroom_ble/bathroom_ble.ino.
private let distanceService = CBUUID(string: "e63f0c82-7f89-4c1e-a4dd-9d87c45b1401")

final class DistanceMonitor: NSObject, CBCentralManagerDelegate {
    private var manager: CBCentralManager!
    private var lastPrint = Date.distantPast

    override init() {
        super.init()
        manager = CBCentralManager(delegate: self, queue: .main)
    }

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            print("Scanning for bathroom sensor BLE distance broadcasts...")
            central.scanForPeripherals(
                withServices: nil,
                options: [CBCentralManagerScanOptionAllowDuplicatesKey: true]
            )
        case .poweredOff:
            print("Turn on Bluetooth in macOS settings.")
        case .unauthorized:
            print("Allow Bluetooth access for this terminal in macOS Privacy & Security settings.")
        case .unsupported:
            print("This Mac does not support Bluetooth Low Energy scanning.")
        case .resetting, .unknown:
            print("Waiting for Bluetooth to become ready...")
        @unknown default:
            print("Unknown Bluetooth state.")
        }
        fflush(stdout)
    }

    func centralManager(
        _ central: CBCentralManager,
        didDiscover peripheral: CBPeripheral,
        advertisementData: [String: Any],
        rssi RSSI: NSNumber
    ) {
        guard let services = advertisementData[CBAdvertisementDataServiceDataKey] as? [CBUUID: Data],
              let data = services[distanceService],
              data.count >= 4,
              data[0] == 1 else {
            return
        }

        // The same reading is advertised repeatedly; show a fresh line about
        // twice a second, even when the measured distance has not changed.
        let now = Date()
        guard now.timeIntervalSince(lastPrint) >= 0.45 else { return }
        lastPrint = now

        let distance = UInt16(data[2]) | (UInt16(data[3]) << 8)
        switch data[1] {
        case 0 where distance != 0xFFFF:
            print("Distance: \(distance) mm  (RSSI \(RSSI) dBm)")
        case 1:
            print("Distance invalid or out of range")
        case 2:
            print("Sensor unavailable; check wiring and power")
        default:
            print("Unknown sensor status: \(data[1])")
        }
        fflush(stdout)
    }
}

let monitor = DistanceMonitor()
RunLoop.main.run()
