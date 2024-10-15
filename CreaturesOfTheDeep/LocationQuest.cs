using UnityEngine;

[CreateAssetMenu(menuName = "Quest/LocationQuest")]
public class LocationQuest : Quest
{
    [SerializeField] private Vector3 targetCoords;
    [SerializeField] private float range = 10;

    public override void StartQuest(QuestSystem qs)
    {
    }

    public override bool IsCompleted(QuestSystem qs)
    {
        return Vector3.Distance(targetCoords, qs.submarineTransform.position) <= range;
    }

    public override Vector3 Debug()
    {
        Gizmos.DrawSphere(targetCoords, 1);
        Gizmos.DrawWireSphere(targetCoords, range);

        return targetCoords;
    }
}