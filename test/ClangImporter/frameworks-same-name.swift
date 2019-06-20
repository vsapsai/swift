// RUN: %empty-directory(%t)

// RUN: echo 'import Includer' > %t/PrivateFrameworksUser.swift
// RUN: %target-swift-frontend(mock-sdk: -sdk %S/Inputs/frameworks-same-name/mock-sdk) -emit-module -o %t %t/PrivateFrameworksUser.swift -F %S/Inputs/frameworks-same-name/extra-frameworks -enable-objc-interop -module-cache-path %t/module-cache-private

// RUN: %target-swift-frontend(mock-sdk: -sdk %S/Inputs/frameworks-same-name/mock-sdk) -emit-module -o %t %s -I %t -enable-objc-interop -module-cache-path %t/module-cache

// Load modules Includee and SameName from mock SDK.
import Includee
// Load modules Includer (extra), Includee, and SameName (extra). Make sure
// they don't conflict with already loaded modules with the same names.
import PrivateFrameworksUser
