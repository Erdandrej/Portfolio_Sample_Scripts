using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

public class QuestSystem : MonoBehaviour
{
    [SerializeField] private List<Quest> quests = new();
    [SerializeField] private Quest currentQuest;

    [SerializeField] private Button newQuestButton;
    [SerializeField] private Button completeQuestButton;

    [SerializeField] public Transform submarineTransform;
    [SerializeField] public ArtefactSystem artefactSystem;
    [SerializeField] public Fax fax;
    [SerializeField] public RaycastingSubmarine sonar;
    [SerializeField] public MalfunctionSystem malfunctionSystem;

    [SerializeField] public UnityEvent questStarted;
    [SerializeField] public UnityEvent questCompleted;

    private float last;

    private void OnEnable()
    {
        newQuestButton.onValueChanged.AddListener(StartNewQuest);
        completeQuestButton.onValueChanged.AddListener(CompleteQuest);
        last = Time.time;
    }

    private void OnDisable()
    {
        newQuestButton.onValueChanged.RemoveListener(StartNewQuest);
        completeQuestButton.onValueChanged.RemoveListener(CompleteQuest);
    }

    private void Update()
    {
        if(currentQuest == null && quests.Count > 0 && last + quests[0].GetResponseTime() <= Time.time)
        {
            StartNewQuest();
        }

        if(currentQuest && currentQuest.IsCompleted(this))
        {
            completeQuestButton.Unblock();
        } else
        {
            completeQuestButton.Block();
        }


        //if (currentQuest && currentQuest.IsCompleted(this))
        //{
        //    questCompleted.Invoke();
        //    Debug.Log("====== Quest was completed! ======");

        //    currentQuest = null;
        //    newQuestButton.Unblock();
        //}
    }

    public void StartNewQuest()
    {
        if (currentQuest == null && quests.Count > 0)
        {
            currentQuest = quests[0];
            quests.RemoveAt(0);
            
            currentQuest.StartQuest(this);
            questStarted.Invoke();

            if (fax) 
            {
                fax.Print(currentQuest.GetQuestName(), currentQuest.GetDescription());
            }
            if (sonar)
            {
                sonar.ArtifactLocation(currentQuest.GetWaypoint());
            }
        }
    }

    public void CompleteQuest()
    {
        if (currentQuest && currentQuest.IsCompleted(this))
        {
            if (fax) fax.Detach();

            questCompleted.Invoke();

            currentQuest = null;
            last = Time.time;
        }
    }

    private void OnDrawGizmos()
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