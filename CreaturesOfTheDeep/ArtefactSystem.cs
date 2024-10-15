using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

public class ArtefactSystem : MonoBehaviour
{
    [SerializeField] private Button pickupButton;

    private readonly List<int> _collectedArtefactIDs = new();

    public Artefact CurrentTargetArtefact { get; private set; }

    [SerializeField] public UnityEvent artefactCollected;

    private void Start()
    {
        pickupButton.Block();
    }

    private void OnEnable()
    {
        pickupButton.onValueChanged.AddListener(CollectArtefact);
    }

    private void OnDisable()
    {
        pickupButton.onValueChanged.RemoveListener(CollectArtefact);
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.gameObject.TryGetComponent(out Artefact artefact) && artefact == CurrentTargetArtefact)
        {
            pickupButton.Unblock();
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (other.gameObject.TryGetComponent(out Artefact artefact) && artefact == CurrentTargetArtefact)
        {
            pickupButton.Block();
        }
    }

    private void OnTriggerStay(Collider other)
    {
        if (other.gameObject.TryGetComponent(out Artefact artefact) && artefact == CurrentTargetArtefact)
        {
            pickupButton.Unblock();
        }
    }

    private void CollectArtefact()
    {
        if (!pickupButton.GetBoolValue()) return;

        pickupButton.Block();
        artefactCollected.Invoke();

        _collectedArtefactIDs.Add(CurrentTargetArtefact.artID);

        Debug.Log("====== Artefact Collected! ======");
        Debug.Log(CurrentTargetArtefact.artName);
        Debug.Log(CurrentTargetArtefact.artDescription);

        Destroy(CurrentTargetArtefact.gameObject);
    }

    public bool WasArtefactCollected(int artefactID)
    {
        return _collectedArtefactIDs.Contains(artefactID);
    }

    public void SetTargetArtefact(int artefactID)
    {
        CurrentTargetArtefact = Artefact.All.Find(a => a.artID == artefactID);
    }
}