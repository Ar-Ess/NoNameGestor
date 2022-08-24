# No Name Gestor
--------------------------------------
No Name Gestor is an application to manage, in a simple and easy way, your actual monetary situation, arrears and future income.
Its main focus is to categorize your total economic amount into small sections to organize your personal expenses and savings.

This project uses SDL loop application system and ImGui as a Graphic library in charge of the UI. It is only built for Windows devices.

No Name Gestor is a high scalable project single-developed, which its main focus is features and usage. Nowadays it lacks the UX and
for sure it will be redesigned.

## Into the gestor
--------------------------------------
The first step is to look at your bank account you want to manage and type your total amount in the **Total Money** field by double clicking it or by draging.
Here is where you'll be updating your money each time you have some movements in your bank account.
Once that is typed, you'll see below a text saying **Unasigned Money**, and the same quantity you've typed in the **Total Money** field.
**Unasigned Money** shows you how many money, from the **Total Money**, you have asigned inside your **Recipients**.

What is a **Recipient**?
It is the way the application allows the user to categorize its money. When we input certain amount of money inside, the **Unasigned Money** label decreases because
we have asigned certain money somewhere. This allows the user to keep track of whether it has asigned too much money to certain recipiens when the **Unasigned Money** field shows in negative nubmers.

You can create a **Recipient** by clicking the "Create" menu on the top left corner and choosing which one or simply by going to the left toolbar and clicking 
on the recipient type you need. // Future explanation of single & plural

Each **Recipient** has a text field to write the category name, a dropdown menu that can be opened with the button labeled ":", an arrow \/ to minimize or maximize it, and a drag & drop symbol "[]" where you can click, drag, and reorder your filters.

Next, what is inside each **Recipient** changes depending of which type you selected. The recipient type can be shown at the left side of the label text.

### Filters
This are the simplest ones. In the **Recipient**, there is a rectangular field showing 0.0, by double clicking or by dragging, you will change its value
to insert money inside that **Recipient**.

## Save & Load
--------------------------------------
On the top left corner of the window, under the "File" menu, there will be 3 options: Save, Save As & Load.
 - Save: Writes back on the loaded file. If there isn't a loaded file, functions as a "Save As".
 - Save As: Opens a file dialog to search for a path to save your file. If a file is loaded, it creates a new one.
 - Load: Opens a file dialog to search for an existent ".nng" file to open in the program.

// Image of the Save/SaveAs/Load feature

### Error: File version different from program's version
This error can occur in 2 situations, which you must check which one it is. 

Open your current No Name Gestor app, go to "About" menu on the top left corner.
There, the first text shown is the name of the app and its version. Keep the version number with you and close your app.
Next, go to the file that generates the error and open it with the NotePad or any other text editor. The first line of text will be "version vX.X". Keep that
version number as well.

Now that you have both verion numbers, go to the section that fits best your situation.

  -> Situation 1: My application version is higher than my file version
  
In this case, open your actual No Name Gestor app and open the file menu on the top left corner. Then, search for the "Old Load" option and select
the Load version of your file. This is loading your file with an old version of the program. If your file is successfully open, directly save it again to
update it to the newest Load system. This way, that file will be opened with the normal "Load" option.

Another problem you might be facing is that the file version does not match with any "Old Load" options. In that case, choose the closer lower option
to your file version. Example:

File Version: v2.3
Program Version: v2.6 -> Version Error
Old Load options:
 - Load v2.0
 - Load v2.2
 - Load v2.4

You have to choose the "Load v2.2", it is the closest lower version to the File Version. "Load v2.4" would not work because is the closest higher version.

// Image of loading older versions
  
  -> Situation 2: My application version is lower than my file version
  
In this situation, you'll have to [download](https://github.com/Ar-Ess/NoNameGestor/releases) the NoNameGestor app that matches your file version. If you want to get a higher application version than your file's one, you'll run into the first situation of this error. Read it to have your application updated at maximum and your file running perfectly!
 
