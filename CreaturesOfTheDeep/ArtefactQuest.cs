using System.Linq;
using UnityEngine;

[CreateAssetMenu(menuName = "Quest/ArtefactQuest")]
public class ArtefactQuest : Quest
{
    [SerializeField] private int targetArtefactID;

    public override void StartQuest(QuestSystem qs)
    {
        qs.artefactSystem.SetTargetArtefact(targetArtefactID);
    }

    public override bool IsCompleted(QuestSystem qs)
    {
        return qs.artefactSystem.WasArtefactCollected(targetArtefactID);
    }

    public override Vector3 Debug()
    {
        var artefact = FindObjectsByType<Artefact>(FindObjectsSortMode.None).ToList().Find(a => a.artID == targetArtefactID);
        if (!artefact) return Vector3.zero;
        Vector3 artefactPosition = artefact.transform.position;

        Gizmos.DrawWireSphere(artefactPosition, 1);

        return artefactPosition;
    }
}