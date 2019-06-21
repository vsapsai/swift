// RUN: %empty-directory(%t)

// RUN: echo 'import Includee' > %t/AlternativeSdkUser.swift
// RUN: %target-swift-frontend(mock-sdk: -sdk %S/Inputs/frameworks-same-name/alternative-sdk) -emit-module -o %t %t/AlternativeSdkUser.swift  -enable-objc-interop -module-cache-path %t/module-cache-alternative

// RUN: %target-swift-frontend(mock-sdk: -sdk %S/Inputs/frameworks-same-name/mock-sdk) -emit-module -o %t %s -I %t -enable-objc-interop -module-cache-path %t/module-cache

// RUN: ls -R %t/module-cache | %FileCheck %s
// CHECK: Includee-{{.+}}.pcm
// CHECK: Includee-{{.+}}.pcm

// Load modules Includee and SameName from mock SDK.
import Includee
// Load module Includee from alternative SDK. Make sure there are no conflicts
// with another module Includee and we have 2 separate modules.
import AlternativeSdkUser
