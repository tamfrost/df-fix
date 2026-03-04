# df-fix ArgoCD Application Helm Chart

This Helm chart creates an ArgoCD Application that deploys the df-fix application from the GitHub repository.

## Prerequisites

- ArgoCD installed in your Kubernetes cluster
- Access to the GitHub repository

## Installation

```bash
# Install the ArgoCD application
helm install df-fix-app .helm/argocd -n argocd

# Or with custom values
helm install df-fix-app .helm/argocd -n argocd -f custom-values.yaml
```

## Configuration

Key values in `values.yaml`:

- `argocd.namespace`: Namespace where ArgoCD is installed (default: `argocd`)
- `argocd.application.name`: Name of the ArgoCD application
- `argocd.application.namespace`: Target namespace for the application
- `argocd.application.source.repoURL`: GitHub repository URL
- `argocd.application.source.targetRevision`: Git branch/tag/commit
- `argocd.application.source.path`: Path to the Helm chart in the repo
- `argocd.application.syncPolicy.automated`: Enable auto-sync

## Customization

Override values in the application by modifying `argocd.application.source.helm.values` in `values.yaml`.
