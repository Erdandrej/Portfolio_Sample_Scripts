using System;
using UnityEngine;

[Serializable]
public abstract class Quest : ScriptableObject
{
    [SerializeField] private string questName;
    [SerializeField] [TextArea(3, 20)] private string description;
    [SerializeField] private float responseTime;

    public string GetQuestName() => questName;
    public string GetDescription() => description;
    public float GetResponseTime() => responseTime;

    public abstract void StartQuest(QuestSystem qs);

    public abstract bool IsCompleted(QuestSystem qs);

    public abstract Vector3 Debug();

    public virtual Vector3 GetWaypoint()
    {
        return new Vector3(10000, 10000, 10000);
    }
}