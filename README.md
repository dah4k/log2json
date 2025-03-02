<!--
Copyright 2025 dah4k
SPDX-License-Identifier: EPL-2.0
-->

# Log2Json

A small program for converting legacy logs to JSON.


## Log Parser State Machine

```mermaid
stateDiagram-v2
    [*] --> StartLogEntry

    StartLogEntry --> InKey : TEXT
    StartLogEntry --> Error

    %% Text Key
    InKey --> InKey : TEXT
    InKey --> EndKey : EQUAL
    InKey --> Error

    %% Text Value
    EndKey --> InValue : TEXT
    InValue --> InValue : TEXT
    InValue --> InKey : SPACE
    InValue --> StartLogEntry : NEWLINE
    InValue --> [*] : EOF
    InValue --> Error

    %% Quoted Text Value
    EndKey --> InQuotedValue : QUOTE
    InQuotedValue --> InQuotedValue : TEXT
    InQuotedValue --> InEscape : BACKSLASH
    InEscape --> InQuotedValue
    InQuotedValue --> EndQuotedValue: QUOTE
    EndQuotedValue --> InKey : SPACE
    EndQuotedValue --> StartLogEntry : NEWLINE
    EndQuotedValue --> [*] : EOF
    EndQuotedValue --> Error
```

