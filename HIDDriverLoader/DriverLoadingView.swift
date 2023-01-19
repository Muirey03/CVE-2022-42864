/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The SwiftUI view that provides the driver loading UI.
*/

import SwiftUI

struct DriverLoadingView: View {

    @ObservedObject var viewModel: DriverLoadingViewModel = .init()

    var body: some View {
        VStack(alignment: .center) {
            Text("Driver Manager")
                .padding()
                .font(.title)
            Text(self.viewModel.dextLoadingState)
                .multilineTextAlignment(.center)
            HStack {
                Button(
                    action: {
                        self.viewModel.activateMyDext()
                    }, label: {
                        Text("Install Dext")
                    }
                )
                Button(
                    action: {
                        self.viewModel.triggerExploit()
                    }, label: {
                        Text("Trigger Exploit")
                    }
                )
            }
        }.frame(width: 500, height: 200, alignment: .center)
    }
}

struct DriverLoadingView_Previews: PreviewProvider {
    static var previews: some View {
        DriverLoadingView()
    }
}
