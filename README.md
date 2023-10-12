# _SmallImapsNotifier_
[![cpp-logo](https://img.shields.io/badge/C++v20-Solutions-blue.svg?style=flat&logo=c%2B%2B)](
https://en.wikipedia.org/wiki/C++
)

Notifies about a new letter on the taskbar, works via the IMAPS: IMAP over SSL/TLS.
Made with the aim of studying the application of State Machine in the real world.

## Features
State Machine bevahior in *plantuml*:

![StateMachine_plantuml](https://github.com/Alex0vSky/SmallImapsNotifier/assets/52796897/1fca7162-ee3d-4921-bde4-03df3403368a)

```
@startuml

[*] --> idle
idle --> TryInbox : CheckAccess / Imap
TryInbox --> terminate : Inavailable [guardAvailable]
TryInbox --> Waiting : StartLoop / InitialEmailCount
TryInbox : entry / InboxAvailable
Waiting --> terminate : HasBreak [guardBreak]
Waiting --> RenewedEmail : RenewEmail / Imap
Waiting : entry / BreakableSleep
RenewedEmail --> terminate : Inavailable [guardAvailable]
RenewedEmail --> Waiting : ContinueLoop
RenewedEmail : entry / (InboxAvailable, EmailCounter)

@enduml
```

Tested on a Gmail account. To get a 16-digit password, you need to enable two-factor authentication for your Gmail account.

@todo:
- [ ] clean up the code
- [ ] get rid opensll dll-s requirements
- [ ] pentesting ssl mitm
- [ ] async UI waiting
- [ ] unit tests
- [ ] CI
- [ ] try a smaller SSL

## Requirements
mailio library, openssl library, boost library.

## Install
The application consists of one file. Require opensll DDL-s.

## Usage
Just run it, in the settings window enter the host and port of the remote IMAP server, and also enter your username and password.
When a new email is received, a balloon notification will appear on your taskbar.
Management via the taskbar: change setting, exit.
The application saves settings in the registry. The login and password will be encrypted before saving.

## Tests
...

## Build
Before building, you must manually install *mailio* and *boost* and change the paths.
*openssl* will be installed via NuGet.

## Contributing
Can ask questions. PRs are accepted. No requirements for contributing.

## Thanks
[boost-ext/sml](https://github.com/boost-ext/sml), *plantuml*, [mailio](https://github.com/karastojko/mailio), boost, openssl

## License
See the [LICENSE](https://github.com/Alex0vSky/SmallImapsNotifier/blob/main/LICENSE) file for license rights and limitations (MIT).
