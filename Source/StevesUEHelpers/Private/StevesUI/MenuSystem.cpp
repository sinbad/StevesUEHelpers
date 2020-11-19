#include "StevesUI/MenuSystem.h"
#include "StevesUI/MenuStack.h"

DEFINE_LOG_CATEGORY(LogMenuSystem)

TWeakObjectPtr<UMenuStack> FMenuSystem::GetHighestFocusPriority()
{
    int Highest = -999;
    TWeakObjectPtr<UMenuStack> Ret;
    
    for (auto && S : ActiveMenuStacks)
    {
        if (S.IsValid() && S->IsRequestingFocus() && S->FocusPriority > Highest)
        {
            Highest = S->FocusPriority;
            Ret = S;
        }
    }
        
    return Ret;
}

void FMenuSystem::MenuStackOpened(UMenuStack* Stack)
{
    UE_LOG(LogMenuSystem, Display, TEXT("MenuStack %s opened"), *Stack->GetName());
    // check to make sure we never dupe, shouldn't normally be a problem
    // but let's just be safe, there will never be that many
    bool bPresent = false;    
    for (auto && S : ActiveMenuStacks)
    {
        if (S.Get() == Stack)
        {
            bPresent = true;
            break;
        }
    }
    if (!bPresent)
        ActiveMenuStacks.Add(Stack);

    if (Stack->IsRequestingFocus())
    {
        auto Highest = GetHighestFocusPriority();
        if (!Highest.IsValid() || Highest->FocusPriority <= Stack->FocusPriority)
        {
            // give new stack the focus if it's equal or higher priority than anything else
            UE_LOG(LogMenuSystem, Display, TEXT("Giving focus to MenuStack %s"), *Stack->GetName());
            Stack->TakeFocusIfDesired();
        }        
    }
}

void FMenuSystem::MenuStackClosed(UMenuStack* Stack)
{
    UE_LOG(LogMenuSystem, Display, TEXT("MenuStack %s closed"), *Stack->GetName());
    
    for (int i = 0; i < ActiveMenuStacks.Num(); ++i)
    {
        if (ActiveMenuStacks[i].Get() == Stack)
        {
            ActiveMenuStacks.RemoveAt(i);
            break;
        }
    }

    // if the menu closing had focus, give it to the highest remaining stack
    if (Stack->HasFocusedDescendants())
    {
        auto Highest = GetHighestFocusPriority();
        if (Highest.IsValid())
        {
            UE_LOG(LogMenuSystem, Display, TEXT("Giving focus to MenuStack %s"), *Highest->GetName());
            Highest->TakeFocusIfDesired();
        }       
    }
}
