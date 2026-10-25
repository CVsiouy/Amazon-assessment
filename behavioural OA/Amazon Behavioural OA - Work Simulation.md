---
title: Amazon Behavioural OA – Work Simulation & Work Style
tags:
  - amazon
  - interview-prep
  - behavioural-oa
  - leadership-principles
assessment_date: 2026-05-09
created: 2026-10-25
source: photos/behavioral (129 photos, IMG_20260509_174419 → IMG_20260509_181536)
---

# Amazon Behavioural OA – Work Simulation & Work Style

> [!info] How to read this note
> - **Rating questions** use a 5-box scale: **1 = Not effective / Not valuable … 5 = Highly effective / Highly valuable**. The number of filled boxes in your photo is the rating you chose.
> - **Your answer** = what the photo shows you selected. *Not captured* = no photo shows a selection for that item.
> - **Recommended** = my best-practice rating, based on Amazon's Leadership Principles. ==Amazon does not publish an answer key==, so treat these as well-reasoned targets, not official answers.
> - **Verdict:** ✅ within 1 point of the recommended rating · ⚠️ 2 points off · ❌ 3 or more points off

## Contents
- [[#Part 1 – Project Fashion Lens]]
  - [[#1.1 Camilla – Code Documentation Feedback]]
  - [[#1.2 Abigail – Code Review]]
  - [[#1.3 Daksh – Progress update (race condition)]]
  - [[#1.4 Camilla – Race condition help]]
  - [[#1.5 Percy – Troubleshooting the API]]
- [[#Part 2 – Project Sound Traveler]]
  - [[#2.1 Ada – Data-tracking options]]
  - [[#2.2 Mai – Button proposal]]
  - [[#2.3 Kyle – Code won't be ready on time]]
  - [[#2.4 Kyle – Final review finds a customer-impacting bug]]
- [[#Part 3 – Retail Website Team]]
  - [[#3.1 Sarah – Urgent bug fix (testing strategy)]]
  - [[#3.2 Lana – Skipping integration tests]]
  - [[#3.3 Kevin – "Quickly approve" after skipping tests]]
  - [[#3.4 Juan – Guardrails against untested deploys]]
- [[#Part 4 – Your Work Style]]
- [[#Key lessons]]

---

## Part 1 – Project Fashion Lens

> [!abstract] Setup (Daksh Jha, Team Lead, Sr. SDE)
> **Fashion Lens** project. Team: **Camilla Garcia** (Sr. SDE, machine learning), **Abigail Katz** (SDE, back end), **Percy Zhu** (SDE, front end).
> **Your role:** develop the API. Run time is the primary requirement and the timeline is tight. Previously developed code is available. You also take part in code reviews.

### 1.1 Camilla – Code Documentation Feedback
> [!quote] Email from Camilla Garcia (Sr. SDE)
> "I just finished reviewing your code documentation. Your documentation could be improved. Here is my high-level feedback:
> - Make sure to be precise in your documentation
> - Provide specifics when noting issues
> - Make sure to detail the severity of each issue"

**Task:** *After thanking Camilla for reviewing your documentation, what would you include in your reply?* (4 questions)

| # | Response | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | "My documentation includes all of your callouts. Would it be helpful if I sent an updated version with comments showing where I already address them?" | Not captured | **1–2** | – |
| 2 | "Can you provide examples where you think I can improve my documentation? This will help me with my updates." | Not captured | **5** | – |
| 3 | *(not photographed)* | – | – | – |
| 4 | *(not photographed)* | – | – | – |

> [!tip] Why
> Camilla's feedback is vague. Asking for concrete examples shows you **Earn Trust** and want to improve. Telling her you already covered everything is defensive.

### 1.2 Abigail – Code Review
> [!quote] Email from Abigail Katz
> Your code solves the run-time requirement but **lacks extensibility** and isn't a long-term solution. The changes needed are fairly complex. She suggests setting up a meeting.

**Task:** *After thanking Abigail for her feedback, what would you include in your reply?* (5 questions)

| # | Response | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | "I agree there are multiple approaches that could be used. I went with the approach I think is best." | 1 | **1** | ✅ |
| 2 | "I don't think you're fully thinking through my approach. I know it won't cause the issues you think it will." | 1 | **1** | ✅ |
| 3 | "Let's meet to discuss your concerns and recommendations, which will help me decide if my approach should be revised." | 5 | **5** | ✅ |
| 4 | "I'll set up a meeting with you and the rest of the Project Fashion Lens team so we can decide as a group what approach to take." | 1 | **2** | ✅ |
| 5 | "The approach needs to provide a long-term solution. Please feel free to make any changes to the code you think are best." | 1 | **1** | ✅ |

> [!tip] Why
> Be open to feedback, discuss it one-on-one, and **keep ownership** of the decision. Don't escalate to the whole team, and don't hand your code over.

> [!note] Follow-up email from Abigail (RE: Code Review)
> She sends her suggestions. They are more complex than the original plan but add value. If you want to implement them, let her know.

### 1.3 Daksh – Progress update (race condition)
> [!quote] Chat from Daksh
> "Hi, can you please send me an update on your progress with the API? Thanks!"

**Context:** You've spent two days on Abigail's suggested changes and are on track for final review, but you just hit an unexpected test failure caused by a **race condition**.
**Task:** *Rate the effectiveness of these as your first, immediate message to Daksh.* (5 questions)

| # | Response | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | "I've run into a race condition and could use your advice to troubleshoot the issue. I'll pull together some additional details and send them to you shortly." | 4 | **5** | ✅ |
| 2 | "I've run into an error and am going to set up a meeting with Abigail to get her help since she suggested the revised approach." | 2 | **2** | ✅ |
| 3 | "I've run into a race condition. I'm going to keep working on this, but may need the team's help during the team meeting in two days." | 1 | **2** | ✅ |
| 4 | "I've run into an issue with the suggested code modifications. I'm going to submit my original code for review so launch won't be delayed." | 1 | **1** | ✅ |
| 5 | "I've run into a race condition and need more time to troubleshoot. I'm not going to meet the final review deadline." | 3 | **2** | ✅ |

> [!tip] Why
> Be transparent right away, ask your lead for help, and follow up with specifics. Don't blame the person who suggested the change, don't sit on the problem, and don't declare a missed deadline before you've asked for help.

### 1.4 Camilla – Race condition help
> [!quote] Email from Camilla
> Daksh asked her to help troubleshoot because she has experience with this. A **global variable** is causing the race condition, and it can be resolved. However, she is **concerned with the approach you've taken** and asks if you have time to discuss today.

**Task:** *What would you include in your reply to Camilla?* (5 questions)

| # | Response | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | "I agree we should discuss. Let me set up a meeting with Daksh, you and me so we can come to a group consensus." | 5 | **4** | ✅ |
| 2 | "Thanks for calling out your concern. Let me know what the approach should be and I'll implement it." | 2 | **2** | ✅ |
| 3 | "Thanks for your advice on the race case. I'll make that fix and then publish the code. I don't think the code needs any additional changes." | 1 | **1** | ✅ |
| 4 | "Thanks for your help with the race case. We just finished doing a thorough review of the approach so there is no need to review this again." | 1 | **1** | ✅ |
| 5 | "Thanks for your review. I don't think we should change the approach but I'll ask Daksh to confirm this." | 1 | **1** | ✅ |

> [!tip] Why
> When a senior engineer raises a design concern, agree to discuss it. Including the team lead is fine because Daksh assigned Camilla. Brushing off the concern, or handing her the decision, both score low.

### 1.5 Percy – Troubleshooting the API
> [!quote] Email from Percy Zhu
> He gets errors when calling the API you developed and thinks it would be solved faster if you helped troubleshoot. He has **already set up time** for you to work together and hopes to resolve it before tomorrow morning's status meeting.
>
> **Chat (RE: Troubleshooting):** "Thanks for your input on the API issue. I'm seeing the same error again. I know you're busy today but can you help me figure out what's wrong? I'm pulling together a list of things I've tried and the results."

**Task:** *What would you include in your reply to Percy?* (5 questions)

| # | Response | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | "Were you able to implement the suggestions I provided? If not, can you work through my suggestions first?" | 1 | **2** | ✅ |
| 2 | "Since my troubleshooting didn't fix the issue, I recommend asking Daksh for help." | 1 | **1** | ✅ |
| 3 | "I have an hour later today to help you. If we can't figure it out, we should involve Daksh." | 5 | **5** | ✅ |
| 4 | "I don't have time right now. Send me your code and I'll revise it later today." | 2 | **1** | ✅ |
| 5 | "I've finished my API call documentation, which may address your issue. If you're still getting an error after working through my documentation, let me know." | 5 | **3** | ⚠️ |

> [!warning] Lesson from Q5
> Pointing Percy to documentation is useful, but it **defers** a teammate who explicitly asked for hands-on help and already booked time. It's your API, so **Ownership** means joining the session. Treat this as a supplement, not the answer, and rate it mid-scale.

---

## Part 2 – Project Sound Traveler

> [!abstract] Setup (Mai King, Team Lead, Sr. SDE)
> **Sound Traveler** is a new Amazon Music feature: customers go from the Music app directly to an artist's web page with one button.
> **Your role:** build the **redirect button**, plus code reviews.
> Team: **Kyle Patterson** (SDE, API and back end), **Ada Jones** (SDE, traffic tracking and customer feedback).

### 2.1 Ada – Data-tracking options
> [!quote] Email from Ada Jones ("Gathering feedback")
> **Option 1: Third-party vendor "Number-House"**
> - Offers all the features needed
> - Integration would be relatively quick
>
> **Option 2: Build our own data-tracking platform**
> - Likely cheaper in the long term
> - More control over future changes
>
> **Additional considerations**
> - Building our own will likely **delay launch**, and launching on time is critical for customers.
> - Next year Amazon Music plans to allow **multiple external links per artist**, including merchandise sold on Amazon.
> - With expected customer growth, Number-House will **get expensive** because it charges by customer volume.
>
> Ada will consolidate everyone's feedback for leadership.

**Task:** *Based on all of the information Ada has provided, what are the key benefits of each option? Rate the value of each benefit.* (Scale: Not valuable → Highly valuable. 4 questions)

| # | Benefit | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | "Number-House would allow us to prioritize an on-time launch." | 4 | **5** | ✅ |
| 2 | "Number-House would eliminate the need to build a custom solution." | 3 | **3** | ✅ |
| 3 | "Building our own data tracking platform is more cost effective in the long-term." | 5 | **5** | ✅ |
| 4 | "Building our own data tracking platform would allow us to customize the platform to meet Sound Traveler's future feature needs." | 5 | **5** | ✅ |

> [!tip] Why
> Rate highest the benefits that map directly to stated facts: launch on time is "critical", cost rises with growth, and multi-link support is planned for next year. "No custom build needed" just restates the option, so it's less valuable.

> [!note] Outcome
> Leadership chose **Number-House** to ensure an on-time launch and will consider an internal build next year.

### 2.2 Mai – Button proposal
> [!quote] Email from Mai
> Start work on the redirect button. Begin by sending a brief **proposal** of how you plan to build it. Ask if you have any questions.

**Task:** *What would you like to ask Mai before writing your proposal?* (5 questions)

| # | Question | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | "Is there any flexibility in the timeline?" | 1 | **1** | ✅ |
| 2 | "Do you have any examples where a similar feature was developed that I can reference?" | 5 | **4** | ✅ |
| 3 | "Which existing systems will the button need to integrate with?" | 5 | **5** | ✅ |
| 4 | "What API will the button need to call?" | 5 | **5** | ✅ |
| 5 | "Should we continue to redirect customers when they select the button if the tracking API fails?" | 5 | **5** | ✅ |

> [!tip] Why
> Good questions clarify requirements, dependencies and failure behaviour that affects customers (**Customer Obsession** and **Dive Deep**). Asking for more time before you've started signals the wrong priorities.

### 2.3 Kyle – Code won't be ready on time
> [!quote] Email from Kyle Patterson ("Feedback for API")
> He has incorporated some of your feedback and will finish the rest later. He **told Mai he's on track**, but he's not sure he can actually finish on time and doesn't want Mai to be concerned yet.

**Task:** *You reviewed Kyle's code and know it won't be ready on time. Rate these actions in response to Kyle's email.* (5 questions)

| # | Action | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | Email the Sound Traveler team and tell them that you have concerns with Kyle's code and timeline. | 1 | **1** | ✅ |
| 2 | Tell Mai that you've reviewed Kyle's code and are concerned based on the amount of changes needed. | 4 | **3** | ✅ |
| 3 | Raise your concerns about Kyle's code during the team meeting later that day. | 2 | **1** | ✅ |
| 4 | Tell Kyle that since it's unlikely he'll be able to meet his deadline, he should inform Mai so the team lead is aware and can provide support and guidance. | 5 | **5** | ✅ |
| 5 | For now, hold off sharing your concerns with the team since it is Kyle's responsibility to provide updates on the status of his code. | 2 | **1** | ✅ |

> [!tip] Why
> Push for transparency, but give Kyle the chance to own it first. Going to Mai directly is acceptable as a second step. Calling Kyle out to the team is not, and neither is staying silent.

### 2.4 Kyle – Final review finds a customer-impacting bug
> [!quote] Email from Kyle ("Requesting final feedback")
> He has incorporated your feedback and Ada's, and **Ada has approved**. Once you approve, he can deploy.

**Task:** *Kyle's updated approach could cause error cases for a **small subset of customers**. Rate these actions.* (5 questions)

| # | Action | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | Ignore the potential issue; it's probably not a problem since Ada has already approved the code. | 1 | **1** | ✅ |
| 2 | Approve the code so it's launched on time, but tell Kyle to build a patch to address your concern after the launch. | 2 | **1** | ✅ |
| 3 | Tell Ada what you identified and ask if she thinks Kyle's code should be revised before it is deployed. | 5 | **3** | ⚠️ |
| 4 | Leave a comment in Kyle's code noting the issue and withhold approval until it is addressed. | 5 | **5** | ✅ |
| 5 | Create a new comment on the code documenting the issue so it can be fixed shortly after launch. | 4 | **2** | ⚠️ |

> [!warning] Lessons
> - **Q3:** You're the reviewer and you found the problem, so raise it **with Kyle** and block the approval. Asking another reviewer to decide hands off your responsibility. It's not harmful, so mid-scale.
> - **Q5:** Knowingly shipping a bug that hits customers, even "a small subset", goes against **Customer Obsession** and **Insist on the Highest Standards**. Anything that says "fix after launch" should be rated low.

---

## Part 3 – Retail Website Team

> [!abstract] Setup (Juan Reyes, Team Lead, Sr. SDE)
> **Your role:** back-end work for website page maintenance, managing updates under Juan's direction, implementing bug fixes, and code reviews.
> Team: **Kevin Li** (SDE, Amazon Basics page purchase features), **Lana Kemper** (SDE, Amazon Appstore page purchase features), **Sarah Klein** (Product Manager, makes sure solutions meet customers' needs).

### 3.1 Sarah – Urgent bug fix (testing strategy)
> [!quote] Email from Sarah Klein ("Bug fix needed")
> A bug on the retail website is affecting a **small subset of customers** but could become a much larger issue at any time. Juan estimated **~3 days to test and fix**, but it seems like an easy fix to Sarah. Can you publish a fix **by end of day**?

**Task:** *Rate the effectiveness of the following **testing strategies**.* (5 questions)

| # | Strategy | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | Write code and ask the team to review it before running tests so you can get immediate feedback and make edits. | 5 | **3** | ⚠️ |
| 2 | Write new unit tests that specifically test the fix you need to make to the code. | 1 | **5** | ❌ |
| 3 | Conduct manual testing then create automated tests as an immediate follow-up. | 5 | **4** | ✅ |
| 4 | Run a subset of tests to spot check code changes; publish the new code if it runs properly. | 5 | **2** | ❌ |
| 5 | Roll back the code to a previous version as a quick fix, and then work on and fully test new code. | 1 | **4** | ❌ |

> [!danger] This was your weakest scenario
> The question asks you to rate **testing strategies**. Pressure to ship fast doesn't make thinner testing "effective".
> - **Unit tests aimed at the fix** are the gold standard: fast, targeted and repeatable. You rated this 1.
> - **Spot-checking a subset and publishing** is exactly the shortcut that causes regressions. You rated this 5.
> - **Rolling back** removes the customer impact immediately and buys time to fix it properly (Customer Obsession plus Highest Standards). You rated this 1. *Caveat: this only applies if a recent change caused the bug, which the scenario implies.*
> - A code review is useful, but it doesn't replace running tests.

### 3.2 Lana – Skipping integration tests
> [!quote] Email from Lana Kemper ("Testing approach for a feature update")
> She's updating a purchase feature on the Amazon Appstore page, and Sarah asked her to move quickly. She's hesitant to skip testing altogether, but plans to do **some manual testing in pre-production and skip new integration tests**. What do you think?

**Task:** *Rate these actions in response to Lana's message.* (5 questions)

| # | Action | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | Remind Lana that automated testing is more likely to catch errors than manual testing. | 5 | **3** | ⚠️ |
| 2 | Inform Juan of the situation so he is aware of Sarah's request and Lana's plan to do limited testing. | 3 | **4** | ✅ |
| 3 | Tell Lana you don't think testing should be skipped and offer to help with the new integration testing. | 5 | **5** | ✅ |
| 4 | Encourage Lana to work with Juan and Sarah to find a compromise between testing timelines and product updates. | 5 | **5** | ✅ |
| 5 | Tell Lana her decision makes sense since Sarah is right about the importance of meeting the deadline. | 1 | **1** | ✅ |

> [!tip] Why
> The best answers **take action**: offer to help, or bring the decision-makers together. A "reminder" is true but passive and doesn't solve her time problem, so rate it mid-scale.

### 3.3 Kevin – "Quickly approve" after skipping tests
> [!quote] Email from Kevin Li ("Code approval for Amazon Basics retail website page")
> Sarah asked him to publish by **tomorrow**. He planned two days of testing but **skipped tests he thinks are unnecessary** so he can launch sooner. He has submitted the code and asks you to review and approve it quickly so he can push.

**Task:** *Rate these actions in response to Kevin's request.* (5 questions)

| # | Action | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | Tell Kevin to reach out to Juan to make sure he considered all the necessary edge cases. | 5 | **4** | ✅ |
| 2 | Send Kevin error reports from previous projects where code was pushed without complete testing. | 1 | **2** | ✅ |
| 3 | Set aside your work and help Kevin run a complete set of tests on his code. | 1 | **3** | ⚠️ |
| 4 | Wait for Lana to leave feedback and provide an approval decision on Kevin's code before providing your feedback and approval decision. | 3 | **1** | ⚠️ |
| 5 | Approve Kevin's code because Sarah is right about the importance of meeting the deadline. | 1 | **1** | ✅ |

> [!warning] Lessons
> - **Q3:** Helping a teammate get proper test coverage before a deadline is a reasonable trade-off (**Ownership** and **Highest Standards**). It costs you your own time, so mid-scale, not 1.
> - **Q4:** As a reviewer, waiting for someone else to decide first is passing the buck. Rate it low.

### 3.4 Juan – Guardrails against untested deploys
> [!quote] Email from Juan ("Additional guardrails")
> There have been a few recent cases of code published without proper testing. Can you help come up with ideas to make sure this doesn't happen again?

**Task:** *Rate the actions you could recommend to help reduce publishing code without proper testing.* (4 questions)

| # | Action | Your answer | Recommended | Verdict |
|---|---|---|---|---|
| 1 | Set up automations that prevent deployments without testing. | 5 | **5** | ✅ |
| 2 | Create a series of Standard Operating Procedures outlining testing requirements. | 5 | **4** | ✅ |
| 3 | Research the root cause of the incidents and share a report of your findings with your team. | 2 | **5** | ❌ |
| 4 | Set up guidelines for troubleshooting code when it's deployed without proper testing. | 1 | **1** | ✅ |

> [!danger] Lesson from Q3
> Root-cause analysis is core Amazon behaviour (**Dive Deep**, the "5 Whys", COE/post-mortems). Finding *why* people skip tests, such as deadline pressure from the PM, is what makes the other fixes stick. Rate it high.

---

## Part 4 – Your Work Style

> [!info] Format
> Untimed (about 6 minutes). Each item pairs two statements. Choose **Most like me / More like me** on the left, or **More like me / Most like me** on the right.
> ==There is no official key.== Amazon is looking for traits that match its Leadership Principles: **Ownership, Bias for Action, Learn and Be Curious, Insist on the Highest Standards, Deliver Results, Adaptability, attention to detail**.
> Each pair usually puts a **hedged or weak** statement ("about as…", "occasionally…", "if I have time…", "when asked to…") against a **strong** one. Pick the strong side. When both sides are strong, either side is fine, and *More like me* is safer.

> [!warning] Your answers here are mostly unconfirmed
> In most Work Style photos **no radio button is filled yet**. The cursor is only hovering over an option. Only #1 and #5 show a confirmed selection. "Your answer" below records where the cursor was.

| # | Left statement | Right statement | Your answer | Recommended | Verdict |
|---|---|---|---|---|---|
| 1 | I like to learn something new every day. | I'll learn new things when I know I'll apply them right away. | **Most – Left** (selected) | Most – Left | ✅ |
| 2 | I often choose predictability over the unknown. | I adapt to change quickly. | Hover: Most – Right, then More – Left (unclear) | Most – Right | ? |
| 3 | It's best to wait for more instruction if you complete tasks early. | I take initiative more than most people, but less than some. | Hover: More – Right | More – Right | ✅ |
| 4 | I'll occasionally make easy-to-fix mistakes. | My team relies on me to check for mistakes. | Hover: Most – Right | Most – Right | ✅ |
| 5 | Most projects need a structured plan. | I usually deal with problems as they come up. | **Most – Left** (selected) | Most – Left | ✅ |
| 6 | I'm about as reliable as everyone else on my team. | I persist until a task is done. | Hover: Most – Left | Most – Right | ❌ |
| 7 | I usually double-check my work. | If I notice something needs to be done, I point it out. | Hover: Most – Left | Either (both strong) | ✅ |
| 8 | I usually make careful plans to avoid problems. | Learning new perspectives helps me generate new ideas. | Hover: Most – Left | Either (both strong) | ✅ |
| 9 | I do my best, even on trivial tasks. | I enjoy keeping a routine about as much as others. | Hover: Most – Right | Most – Left | ❌ |
| 10 | I'm the first to notice an error. | I am proactive. | Hover: More – Left | Either (both strong) | ✅ |
| 11 | My learning interests are varied. | I usually create a plan before starting my work. | Hover: Most – Right | Either (both strong) | ✅ |
| 12 | I occasionally like to try new approaches. | I can always be trusted to fulfill my obligations. | Hover: More – Right | Most – Right | ✅ |
| 13 | I seek out work that needs to be done. | I notice errors that others often miss. | Hover: More – Right | Either (both strong) | ✅ |
| 14 | I always prefer working from a plan. | I like applying my existing knowledge to new areas. | Hover: Most – Right | Either (both strong) | ✅ |
| 15 | I rarely miss deadlines. | I like routine, but also enjoy the occasional change. | Hover: More – Left | Most – Left | ✅ |
| 16 | I take initiative when asked to. | It frustrates me when things aren't perfect. | Hover: More – Left | More – Right | ⚠️ |
| 17 | I follow plans diligently. | Learning new things gives me more opportunities. | Hover: Most – Right | Either (both strong) | ✅ |
| 18 | Change can be good, but can create tension. | I would never miss a deadline without a good reason. | Hover: More – Left | Most – Right | ❌ |
| 19 | People would describe me as detail oriented. | I sometimes accept guidance from others. | Hover: Most – Right | Most – Left | ❌ |
| 20 | I enjoy learning new things from time to time. | I produce my best work when working from a detailed plan. | Hover: Most – Left | Most – Right | ❌ |
| 21 | There can be good reasons to change a routine. | I follow through on my obligations. | Hover: Most – Right | Most – Right | ✅ |
| 22 | People tell me I have a keen eye for detail. | I take action when I notice it's needed. | Hover: More – Left | Either (both strong) | ✅ |
| 23 | I like learning about new topics if I have time. | The key to completing tasks is having a detailed plan. | Hover: More – Left | Most – Right | ❌ |
| 24 | I include some flexibility in my routine. | I'm known for getting work done. | Hover: More – Right | Most – Right | ✅ |
| 25 | I take initiative in the areas I'm well-informed about. | I catch more mistakes than most. | Hover: More – Left | Most – Right | ❌ |
| 26 | I like to learn new things when I have extra time. | Making a detailed plan jumpstarts my momentum. | Hover: More – Left | Most – Right | ❌ |
| 27 | I try to complete my tasks before I end my day. | I like to understand the reasons for a change. | Hover: More – Left | Either (Left leans Ownership) | ✅ |
| 28 | When my work is complete, I look for more work. | I'm the first to notice when things change. | Hover: More – Left | Either (Left leans Ownership) | ✅ |
| 29 | I am a life-long learner. | I always plan and prioritize my work tasks. | Hover: Most – Left | Either (both strong) | ✅ |
| 30 | People tell me I'm dependable. | I take a flexible approach to new challenges. | Hover: More – Left | Either (both strong) | ✅ |
| 31 | I usually influence people by providing details. | I'm as much of a self-starter as anyone else. | Hover: More – Left | Most – Left | ✅ |
| 32 | I like applying my current skills to new challenges. | A plan increases efficiency. | Hover: Most – Left | Either (both strong) | ✅ |
| 33 | Sometimes I work hard, and sometimes I hardly work. | I seldom work outside of my comfort zone. | Hover: More – Left | More (either; both weak) | ✅ |
| 34 | I sometimes miss an error I probably should have caught. | I wait for clear instruction before trying something new. | Hover: More – Left | More (either; both weak) | ✅ |
| 35 | Acquiring knowledge from different areas helps me solve problems. | It makes me feel good when a plan works out. | Hover: Most – Left | Either (both strong) | ✅ |
| 36 | Typically, I am disciplined about getting work done. | Change is good but can sometimes be disruptive. | Hover: Most – Left | Most – Left | ✅ |

> [!tip] Pattern to remember
> Several of your ❌ picks chose a **hedged learning statement** ("from time to time", "if I have time", "when I have extra time") over a **strong planning, reliability or detail statement**. Hedged wording is the signal that a statement is the weaker choice. Avoid it even when the topic (learning) sounds attractive.

---

## Key lessons

> [!summary] What the Work Simulation rewards
> 1. **Own it and be transparent early.** Tell your lead about blockers right away and ask for help (Daksh 1.3, Kyle 2.3).
> 2. **Never trade customer impact for a deadline.** Block approval of known bugs, roll back, and don't "patch after launch" (2.4, 3.1, 3.3).
> 3. **Testing is not optional.** Targeted unit and integration tests beat spot checks. Automated gates and root-cause analysis prevent repeats (3.1, 3.2, 3.4).
> 4. **Respond to feedback with curiosity, not defence.** Ask for examples and meet to discuss (1.1, 1.2, 1.4).
> 5. **Help peers directly, but don't take over their work** (Percy 1.5, Lana 3.2, Kevin 3.3).
> 6. **Escalate proportionately.** Talk to the person first, then the lead. Never call someone out to the whole team (2.3).
> 7. **Ask clarifying questions about requirements, integrations and failure modes**, not about timeline slack (2.2).

> [!check] Score snapshot (rating questions you were photographed answering)
> - ✅ 47 · ⚠️ 7 · ❌ 4 (58 rated items). The ❌s are Sarah Q2, Q4, Q5 and Juan Q3, all about **testing discipline**.
> - Strongest areas: handling feedback, communicating with your lead, Mai's requirement questions, Ada's trade-off analysis.
> - Focus area: **testing and quality under deadline pressure**.
