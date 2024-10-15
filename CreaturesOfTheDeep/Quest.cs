using System;
using UnityEngine;

[Serializable]
public abstract class Quest : ScriptableObject
{
    [SerializeField] private string questName;
    [SerializeField] [TextArea] private string description;

    public string GetQuestName() => questName;
    public string GetDescription() => description;

    public abstract void StartQuest(QuestSystem qs);

    public abstract bool IsCompleted(QuestSystem qs);

    public abstract Vector3 Debug();
}