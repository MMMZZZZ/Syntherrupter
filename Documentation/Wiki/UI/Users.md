# User Settings

[Back to the User Interface Reference page](README.md#readme)

Syntherrupter has three different built-in user accounts. You can control the limits and settings for each user on this page. 

## Index
* [Index](#index)
* [What you see](#what-you-see)
* [What you get](#what-you-get)
	* [User Rights](#user-rights)
	* [Table Layout](#table-layout)
	* [Name](#name)
	* [Password](#password)
		* [Auto Login](#auto-login)
	* [Max. Ontime, Max. BPS, Max. Duty](#max-ontime-max-bps-max-duty)
	* [Return](#return)

## What you see

![User Settings](/Documentation/Pictures/UI/Users.png)

## What you get

### User Rights

The whole point of having different users is to limit the rights and possibilities such that safe operation is guaranteed. The following table gives you an overview about the purpose and rights of every user

|  | **User 0 (Guest)** | **User 1 (Normal)** | **User 2 (Admin)** |
|---|---|---|---|
| **Role** | Guests/Kids that may not know what they are doing. Can't do anything wrong. | Every day operation within the coil limits.  | Allowed to do everything.  |
| **Access to Settings** | No (Since v4.1.0) | Limited | Yes |
| **Settings: Serial Passthrough / Firmware Update** | N/A  | No | Yes |
| **Settings: Coil Limits** | N/A | No | Yes |
| **User Limits: View/Change User 0** | N/A | Yes | Yes |
| **User Limits: View User 1** | N/A | Yes | Yes |
| **User Limits: Change User 1** | N/A | Only name and password | Yes |
| **User Limits: View/Change User 2** | N/A | No | Yes, [except ontime and duty settings](#max-ontime-max-bps-max-duty) |


### Table Layout

Each row contains another property of the users, as described on the left. Each column represents one of the three users. The leftmost column contains the settings of user 0 ("Guest"), the middle one is for user 1 ("Normal"), and the rightmost column controls user 2 ("Admin").

Each field of the table can be selected by a simple touch, and modified with the keypad. Touching the name or password fields opens a fullsize keyboard. 

### Name

The name field allows you to give each user a more appealing name than *user 1/2/3*. The name can be anything the keyboard allows you to type. The only restriction is that it needs to be at least 1 and at most 16 characters long.

### Password  

**Important: Enter only digits (0-9) as password!** While the keypad on the Login page only allows you to enter numbers only, the settings here actually open the full keyboard. This is a limitation of the current implementation of the entire password stuff. Solving it is possible and not really hard, but it does take some time. Honestly, since the workaround is not hard at all *and* most users won't even face the issue, I had no motivation so far to fix this. 

That aside, your password can have any length from 1-16. 

Note: if you give two users the same password, you'll get logged in as the user with *the least* rights. Don't use the same password for multiple accounts. Not on Syntherrupter and nowhere else either.

#### Auto Login

If you set the password of a user to `0`, Syntherrupter will automatically log in to that user on startup. This is for the users that don't require any sort of password protection. You can still log in to the other two accounts using the [Switch User button](Menu.md#switch-user) in the Main Menu.

### Max. Ontime, Max. BPS, Max. Duty

These value specify the *slider ranges* in the UI. They are independant of the [Coil Limits](Coil%20Limits.md#readme)! 

The only sort of exception are the ontime and duty settings for user 2. Since user two is supposed to have full rights, and since at the same time the coil limits are "hard limits", it makes sense for user 2 to be allowed to use the full range up to the coil limits - but no further. So they're set automatically to the highest of you coil limits. Since there's no coil limit for the BPS, that value can still be edited. 

### Return

Applies the settings and brings you back to the [Settings Menu](Settings.md#readme).
