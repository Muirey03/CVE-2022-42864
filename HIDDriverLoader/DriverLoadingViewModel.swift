/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The view model that indicates the state of driver loading.
*/

import Foundation
import SystemExtensions
import os.log
import IOKit

class DriverLoadingStateMachine {

    enum State {
        case unloaded
        case activating
        case needsApproval
        case activated
        case activationError
    }

    enum Event {
        case activationStarted
        case promptForApproval
        case activationFinished
        case activationFailed
    }

    static func process(_ state: State, _ event: Event) -> State {

        switch state {
        case .unloaded:
            switch event {
            case .activationStarted:
                return .activating
            case .promptForApproval, .activationFinished, .activationFailed:
                return .activationError
            }

        case .activating, .needsApproval:
            switch event {
            case .activationStarted:
                return .activating
            case .promptForApproval:
                return .needsApproval
            case .activationFinished:
                return .activated
            case .activationFailed:
                return .activationError
            }

        case .activated:
            switch event {
            case .activationStarted:
                return .activating
            case .promptForApproval, .activationFailed:
                return .activationError
            case .activationFinished:
                return .activated
            }

        case .activationError:
            switch event {
            case .activationStarted:
                return .activating
            case .promptForApproval, .activationFinished, .activationFailed:
                return .activationError
            }
        }
    }
}

class DriverLoadingViewModel: NSObject {

    // Your dext may not start in unloaded state every time. Add logic or states to check this.
    @Published private var state: DriverLoadingStateMachine.State = .unloaded

    private let dextIdentifier: String = "com.muirey03.hiddriverpoc.driver"

    public var dextLoadingState: String {
        switch state {
        case .unloaded:
            return "HIDDriver isn't loaded."
        case .activating:
            return "Activating HIDDriver, please wait."
        case .needsApproval:
            return "Please follow the prompt to approve HIDDriver."
        case .activated:
            return "HIDDriver has been activated and is ready to use."
        case .activationError:
            return "HIDDriver has experienced an error during activation.\nPlease check the logs to find the error."
        }
    }
}

extension DriverLoadingViewModel: ObservableObject {

}

extension DriverLoadingViewModel {

    func activateMyDext() {
        activateExtension(dextIdentifier)
    }

    func triggerExploit() {
        sploit()
    }

    /// - Tag: ActivateExtension
    func activateExtension(_ dextIdentifier: String) {

        let request = OSSystemExtensionRequest
            .activationRequest(forExtensionWithIdentifier: dextIdentifier,
                               queue: .main)
        request.delegate = self
        OSSystemExtensionManager.shared.submitRequest(request)

        self.state = DriverLoadingStateMachine.process(self.state, .activationStarted)
    }
    
    // This method isn't used in this example, but is provided for completeness.
    func deactivateExtension(_ dextIdentifier: String) {

        let request = OSSystemExtensionRequest.deactivationRequest(forExtensionWithIdentifier: dextIdentifier, queue: .main)
        request.delegate = self
        OSSystemExtensionManager.shared.submitRequest(request)

        // Update your state machine with deactivation states and process that change here
    }
}

extension DriverLoadingViewModel: OSSystemExtensionRequestDelegate {

    func request(_ request: OSSystemExtensionRequest,
                 actionForReplacingExtension existing: OSSystemExtensionProperties,
                 withExtension ext: OSSystemExtensionProperties) -> OSSystemExtensionRequest.ReplacementAction {

        var replacementAction: OSSystemExtensionRequest.ReplacementAction

        os_log("sysex actionForReplacingExtension: %@ %@", existing, ext)

        // Add appropriate logic here to determine if the extension should be
        // replaced by the new extension. Common things to check for include
        // testing whether the new extension's version number is newer than
        // the current version number, or that the bundleIdentifier has changed.
        // For simplicity, this sample always replaces the current extension
        // with the new one.
        replacementAction = .replace

        // The upgrade case may require a separate set of states.
        self.state = DriverLoadingStateMachine.process(self.state, .activationStarted)

        return replacementAction
    }

    func requestNeedsUserApproval(_ request: OSSystemExtensionRequest) {

        os_log("sysex requestNeedsUserApproval")

        self.state = DriverLoadingStateMachine.process(self.state, .promptForApproval)
    }

    func request(_ request: OSSystemExtensionRequest, didFinishWithResult result: OSSystemExtensionRequest.Result) {

        os_log("sysex didFinishWithResult: %d", result.rawValue)

        // The "result" may be "willCompleteAfterReboot", which would require another state.
        // This sample ignores this state for simplicity, but a production app should check for it.

        self.state = DriverLoadingStateMachine.process(self.state, .activationFinished)
    }

    func request(_ request: OSSystemExtensionRequest, didFailWithError error: Error) {

        os_log("sysex didFailWithError: %{public}@", error.localizedDescription)

        // Some possible errors to check for:
        // Error 4: The dext identifier string in the code needs to match the one used in the project settings.
        // Error 8: Indicates a signing problem. During development, set signing to "automatic" and "sign to run locally". See README.md for more.

        // While this app only logs errors, production apps should provide feedback to customers about any errors encountered while loading the dext.

        self.state = DriverLoadingStateMachine.process(self.state, .activationFailed)
    }
}
