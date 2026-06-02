# Commit Nature
Emojis should help reviewers to quickly visually and identify the nature of the commit.
For clear visual identification start the commit message with an applicable emoji :

✨ :sparkles: Introducing new features
🎨 :art: Improving structure/format of the code.
⚡ :zap: Improving performance.
🔥 :fire: Removing code or files.
🐛 :bug: Fixing a bug
🚑 :ambulance: Critical hotfix
📝 :memo: Writing Docs.
🚀 :rocket: Updating the UI and styles files.
🔒 :lock: Fixing security issues.
✅ :white_check_mark: Releasing/ Version tags.
🚧 :construction: Work in progress.
💚 :green_heart: Fixing CI Build
⬇️ :arrow_down: Downgrading dependencies
⬆️ :arrow_up: Upgrading dependencies
👷 :construction_worker: Adding CI build system.
📈 :chat_with_upwards_trend: Adding analytics tracking code.
♻️ :recycle: Refactoring code.
➖ :heavy_minus_sign: Removing a dependency.
➕ :heavy_plus_sign: Adding a dependency.
🔧 :wrench: Changing configuration files.
🌐 :globe_with_meridians: Internalization and localization
⏪ :rewind: Reverting changes.
👌 :ok_hand: Updating code due to code review changes.
🏗️ :building_construction: Making architectural changes
🧪 Tests

# Git Commit Messages
Git Commit Messages should help reviewers to do better reviews.

- Write short messages ( 72 characters or less )
- Use the present tense 
- Use the imperative mood
- Start your commit with feat(components_Name) and :emoji-to-use: to make the nature of your commit clear

Preferred :
- feat(views) ✨ add start simulation button 
- feat(controllers) ✨ add network routes controller

Not Preferred : 
- add start simulation button
- add network routes controller

# Branches Naming
Branch name should be more dependent on some combination like :
( Task Type, Name of feature, Jira Ticket number & Subtask ) so what are this meaning ?

As an example: Task Type / Name of Feature / Jira Ticket Number - SubTask

👉 Feature/MAP/NET-06-AddSimulationButton
👉 Feature/SIMULATION/NET-12-AddMaxCarsNumberForSimulations
👉 Improvement/MAP/NET-05-ImproveMapLoadingTime

1. Task Type -> Indicates the type of ticket you are working on.
   (Feature/ Bug/ Refactoring/ hotFix)
2. Name of Feature -> example map, simulation, routes...
3. Jira ticket number -> including ticket number makes a clear visibility of tracking Branch commits & Prs in the ticket description.