using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

public class QuestSystem : MonoBehaviour
{
    [SerializeField] private List<Quest> quests = new();
    [SerializeField] private Quest currentQuest;
    [SerializeField] private Button newQuestButton;

    [SerializeField] public Transform submarineTransform;
    [SerializeField] public ArtefactSystem artefactSystem;

    [SerializeField] public UnityEvent questStarted;
    [SerializeField] public UnityEvent questCompleted;

    private void OnEnable()
    {
        newQuestButton.onValueChanged.AddListener(StartNewQuest);
    }

    private void OnDisable()
    {
        newQuestButton.onValueChanged.RemoveListener(StartNewQuest);
    }

    private void Update()
    {
        if (currentQuest && currentQuest.IsCompleted(this))
        {
            questCompleted.Invoke();
            Debug.Log("====== Quest was completed! ======");

            currentQuest = null;
            newQuestButton.Unblock();
        }
    }

    private void StartNewQuest()
    {
        if (newQuestButton.GetBoolValue() && currentQuest == null && quests.Count > 0)
        {
            currentQuest = quests[0];
            quests.RemoveAt(0);
            newQuestButton.Block();
            
            currentQuest.StartQuest(this);
            questStarted.Invoke();
            
            Debug.Log("====== Quest was started! ======");
            Debug.Log("Quest Name: " + currentQuest.GetQuestName());
            Debug.Log("Description " + currentQuest.GetDescription());
        }
    }

    private void OnDrawGizmosSelected()
    {
        var previous = transform.position;

        if (currentQuest)
        {
            Gizmos.color = Color.green;
            var temp = currentQuest.Debug();
            Gizmos.DrawLine(previous, temp);
            previous = temp;
        }

        Gizmos.color = Color.cyan;
        foreach (Quest quest in quests)
        {
            var temp = quest.Debug();
            Gizmos.DrawLine(previous, temp);
            previous = temp;
        }
    }
}